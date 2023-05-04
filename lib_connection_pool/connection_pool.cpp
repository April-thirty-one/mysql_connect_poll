#include "./connection_pool.h"
#include <json/reader.h>
#include <json/value.h>
#include <iostream>

/**
 * @brief ConnectionPool 构造函数创建一个连接数最少的连接池，并启动两个线程来产生和回收连接。
 *        如果 parseJsonFile() 函数返回 false，构造函数将返回而不创建任何连接。
 *        如果函数返回“true”，构造函数将创建“min_size”个连接，并启动两个线程来监控和管理连接池。但是，构造函数本身不返回任何内容。
 */
ConnectionPool::ConnectionPool()
{
#if 0
// Json 文件加载不上
    // 加载配置文件
    if (!this->parseJsonFile()) {
       std::cout << "[hint]: Failed to load the configuration file" << std::endl;
       return;
    }
#else

    this->ip = "192.168.43.198";
    this->user = "admin";
    this->password = "cyp774940";
    this->db_name = "ChengYaPeng";
    this->port = 3306;
    this->min_size = 4;
    this->max_size = 20;
    this->timeout = 1000;
    this->max_idle_time = 5000;
#endif

    // 为连接池创建最小连接数个连接
    for (int i = 0; i < this->min_size; i++) {
        this->addConection();
    }

    // 检测连接池中的连接够不够用，不够用就新建连接
    std::thread producer(&ConnectionPool::produceConnection, this);

    // 检查是否连接过剩，删除连接
    std::thread recycler(&ConnectionPool::recycleConnection, this);

    // 这两个线程要时时监控连接池，不可以使用 join(), 要将子线程和主线程分离，使用 detach()
    producer.detach();
    recycler.detach();
}

/**
 * @brief 该函数返回 ConnectionPool 对象的单例实例。
 * 
 * @return 返回指向 ConnectionPool 对象的指针。具体来说，它返回一个指向 ConnectionPool
 *         静态实例的指针，该实例在程序的生命周期内只创建一次。
 */
ConnectionPool * ConnectionPool::getConnectionPool()
{
    static ConnectionPool * connection_pool = new ConnectionPool();
    return connection_pool;
}

/**
 * @brief 该函数解析包含数据库配置信息的 JSON 文件，并在 ConnectionPool 对象中设置相应的变量。
 * 
 * @return 返回一个布尔值。如果成功解析 JSON 文件并提取必要的值，则为 true，否则为 false。
 */
 // !!!!!!!!!!!!!!!!! 返回 false 有问题
bool ConnectionPool::parseJsonFile()
{
    std::ifstream ifs("./db_conf.json");
    Json::Reader rd;
    Json::Value val;
    rd.parse(ifs, val);
    if (val.isObject()) {
        this->ip = val["ip"].asString();
        this->port = val["port"].asInt();
        this->user = val["user_name"].asString();
        this->password = val["password"].asString();
        this->db_name = val["db_name"].asString();
        this->min_size = val["min_size"].asInt();
        this->max_size = val["max_size"].asInt();
        this->max_idle_time = val["max_idle_time"].asInt();
        this->timeout = val["timeout"].asInt();

        return true;
    } 
    return false;
}

/**
 * @brief 当连接池中的连接数量不够时，此函数生成新连接并将它们添加到连接池中，同时确保维持池的最小大小。
 */
void ConnectionPool::produceConnection()
{
    while (true) {
        std::unique_lock<std::mutex> locker(this->mutex);
        while (this->connection_queue.size() >= this->min_size) {     
            // 这里使用 while 而不是 if 是为了防止多线程同时满足条件，导致多创建连接，但是在这里我们只有一个生产连接的线程，所以不会出现这样的问题
            this->cond.wait(locker);
        }
        this->addConection();

        // 生产连接后，同时因为连接数量不够2️而等待的线程
        this->cond.notify_all();
    }
}

/**
 * @brief 在连接数量大于最小数量的情况下，释放多余的（空闲时间超过 this->max_idle_time）连接
 */
void ConnectionPool::recycleConnection()
{
    while (true) {
        // 因为这个方法可以一次将所有过剩的连接全部销毁，所以没必要不断检查，可以有一个时间间隔的休息
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard<std::mutex> locker(this->mutex);
        while (this->connection_queue.size() > this->min_size) {
            // 我们的队列是单向队列，所以最空闲的连接一定是先进入队列的队头位置
            MySQLConnect * connection = this->connection_queue.front();
            if (connection->getAliveTime() > this->max_idle_time) {
                this->connection_queue.pop();
                delete connection;
            }
            else {
                break;
            }
        }
    }
}

/**
 * @brief 该函数向连接池中添加一个新的 MySQL 连接。
 */
void ConnectionPool::addConection()
{
    // 创建一个连接，并放入连接池中
    MySQLConnect* connection = new MySQLConnect;
    bool flag = connection->connect(this->user, this->password, this->db_name, this->ip, this->port);
    if (!flag) {
        std::cout << "[hint]: in " << __func__ << ", connect failed" << std::endl;
    }
    connection->refreshAliveTime();
    this->connection_queue.push(connection);
}

/**
 * @brief 该函数从连接池返回指向 MySQL 连接的共享指针，并在不再需要时自动将连接回收回连接池。
 * 
 * @return 指向 MySQLConnect 对象的共享指针，表示与 MySQL 数据库的连接。共享指针用于在不再需要时自动将连接返回到连接池。
 */
std::shared_ptr<MySQLConnect> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> locker(this->mutex);
    while (this->connection_queue.empty()) {
        if (std::cv_status::timeout == this->cond.wait_for(locker, std::chrono::milliseconds(this->timeout))) { 
            // 表示等待一段时间后还是没有得到连接 -- 处理：返回一个 nullptr，表示失败，或者继续等待
            if (this->connection_queue.empty()) {
                continue;
            }
        }
    }

    // 连接池中存在可用连接
    // 本方法的返回值之所以是 shared_ptr 是为了实现自动回收连接到连接池中, 这需要使用 shared_ptr<>() 的第二个参数，名为删除器
    std::shared_ptr<MySQLConnect> connection(this->connection_queue.front(), [this](MySQLConnect * conn) -> void {
        std::lock_guard<std::mutex> locker(this->mutex);
        conn->refreshAliveTime();
        this->connection_queue.push(conn);
    });
    this->connection_queue.pop();

    // 当消费者取出连接后，可能连接池中的连接数量小于 this->min_size，所以要通知生产者线程进行判断
    this->cond.notify_all();    // 本程序的逻辑比较简单，所以生产者和消费者使用的是同一个 condition，改进可以变为两个

    return connection;
}

/**
 * @brief ConnectionPool 类的析构函数，删除其连接队列中的所有 MySQLConnect 对象。
 */
ConnectionPool::~ConnectionPool()
{
    while (!this->connection_queue.empty()) {
        MySQLConnect * connection = this->connection_queue.front();
        this->connection_queue.pop();
        delete connection;
    }
}