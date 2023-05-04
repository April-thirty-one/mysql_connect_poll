#include <chrono>
#include <iostream>
#include "../lib_connection_pool/connection_pool.h"


/**
 * @brief 该函数连接到 MySQL 数据库、插入数据、查询数据并打印结果。
 * 
 * @return 成功返回整数值 0, 失败返回 -1。
 */
int query()
{
    // 创建数据库连接
    MySQLConnect connection;
    bool flag = connection.connect("admin", "cyp774940", "ChengYaPeng", "192.168.43.198", 3306);
    if (!flag) {
        std::cout << "[hint]: connect database failed" << std::endl;
        return -1;
    }

    // 进行插入操作
    std::string sql = "insert into account (name, money) values('cyp', 200000)";
    flag = connection.update(sql);
    if (!flag) {
        std::cout << "[hint]: insert database failed" << std::endl;
    }

    // 查询数据
    sql = "select * from account";
    flag = connection.query(sql);
    if (!flag) {
        std::cout << "[hint]: query database failed" << std::endl;
    }

    // 遍历数据表中的字段
    while (connection.next()) {
        std::cout << connection.getValue(0) << "  "
                  << connection.getValue(1) << "  "
                  << connection.getValue(2) << std::endl;
    }  
    
    return 0;
}

// ================================================= // 
/// 性能测试
/// 测试一：单线程：使用/不使用连接池
/// 测试二：多线程：使用/不使用连接池
// ================================================= // 

/**
 * @brief 该函数重复连接到 MySQL 数据库并向其中插入一条记录。
 * 
 * @param begin 循环的起始索引。
 * @param end end参数是循环的上限，表示循环将执行的次数。
 */
void op1(int begin, int end)
{
    for (int i = begin; i < end; i++) {
        MySQLConnect connection;
        bool flag = connection.connect("admin", "cyp774940", "ChengYaPeng", "192.168.43.198", 3306);
        if (!flag) {
            std::cout << "[hint]: query database failed" << std::endl;
        }
        std::string sql = "insert into account (name, money) values ('cyp', 1000000)";
        flag = connection.update(sql);
        if (!flag) {
            std::cout << "[hint]: insert database failed" << std::endl;
        }
    }
}

/**
 * @brief 该函数执行一个循环，从连接池获取连接并将数据插入 MySQL 数据库。
 * 
 * @param pool 指向 ConnectionPool 对象的指针，该对象管理数据库连接池。
 * @param begin 用于迭代一系列值的循环的起始索引。
 * @param end 结束参数是一个整数值，表示循环的结束索引。循环从开始索引迭代到结束索引（不包括）。
 */
void op2(ConnectionPool * pool, int begin, int end) {
    for (int i = begin; i < end; i++) {
        std::shared_ptr<MySQLConnect> connection = pool->getConnection();
        if (connection == nullptr) {
            std::cout << "[hint]: Acquire connection failed" << std::endl;
        }
        std::string sql = "insert into account (name, money) values ('cyp', 1000000)";
        bool flag = connection->update(sql);
        if (!flag) {
            std::cout << "[hint]: insert database failed" << std::endl;
        }
    }
}

/**
 * @brief 该函数测试在单个线程中不使用连接池与使用连接池的性能。
 */
void test_single_thread()
{
#if 0
    // 不使用连接池，单线程，用时: 51899062283 纳秒 51899 毫秒
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    op1(0, 5000);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - start;
    std::cout << "不使用连接池，单线程，用时: " << length.count() << "纳秒"
                                            << length.count() / 1000000 << "毫秒"
                                            << std::endl;
#else
    // 使用连接池，单线程，用时: 23257899240 纳秒 23257毫秒
    ConnectionPool * pool = ConnectionPool::getConnectionPool();
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    op2(pool, 0, 5000);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - start;
    std::cout << "使用连接池，单线程，用时: " << length.count() << "纳秒"
                                           << length.count() / 1000000 << "毫秒" 
                                           << std::endl;
#endif
}

/**
 * @brief 该函数测试在单个线程中不使用连接池与使用连接池的性能。
 */
void test_multi_thread() {
#if 0
    // 不使用连接池，多线程，用时: 16591200961 纳秒 16591 毫秒
    MySQLConnect connection;    
    bool flag = connection.connect("admin", "cyp774940", "ChengYaPeng", "192.168.43.198");
    if (!flag) {
        std::cout << "[error]: in " << __func__ << ", connect failed" << std::endl;
    }
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::thread td1(op1, 0, 1250);
    std::thread td2(op1, 1250, 2500);
    std::thread td3(op1, 2500, 3750);
    std::thread td4(op1, 3750, 5000);

    td1.join();
    td2.join();
    td3.join();
    td4.join();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - start;
    std::cout << "不使用连接池，多线程，用时: " << length.count() << "纳秒"
                                            << length.count() / 1000000 << "毫秒" << std::endl;
#else
    // 使用连接池，多线程，用时:    6606297835 纳秒  6606 毫秒
    ConnectionPool * pool = ConnectionPool::getConnectionPool();
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::thread td1(op2, pool, 0, 1250);
    std::thread td2(op2, pool, 1250, 2500);
    std::thread td3(op2, pool, 2500, 3750);
    std::thread td4(op2, pool, 3750, 5000);

    td1.join();
    td2.join();
    td3.join();
    td4.join();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto length = end - start;
    std::cout << "  使用连接池，多线程，用时: " << length.count() << "纳秒"
              << length.count() / 1000000 << "毫秒" << std::endl;
#endif
}

/**
 * main函数调用test_multi_thread函数，返回0。
 * 
 * @param argc 传递给程序的命令行参数的数量，包括程序本身的名称。
 * @param argv `argv` 参数是一个字符串数组，表示传递给程序的命令行参数。第一个元素（`argv[0]`）是程序本身的名称，后面的元素（`argv[1]` 到
 * `argv[argc-1]`）是任何附加的
 * 
 * @return 主函数返回一个整数值 0。
 */
int main(int argc, char * argv[])
{
    test_multi_thread();
    return 0;
}