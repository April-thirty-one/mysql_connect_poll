#ifndef LIB_CONNECTION_POOL_CONNECTION_POOL_H__
#define LIB_CONNECTION_POOL_CONNECTION_POOL_H__

#include "../lib_mysql_connect/mysql_connect.h"

#include <iostream>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <json/json.h>
#include <thread>

class ConnectionPool
{
public:
    ConnectionPool(ConnectionPool & other) = delete;
    ConnectionPool(const ConnectionPool & other) = delete;
    ConnectionPool & operator = (const ConnectionPool & other) = delete;
    ~ConnectionPool();

    static ConnectionPool * getConnectionPool();
    std::shared_ptr<MySQLConnect> getConnection();     // 消费者 -- 获取连接池中一个可用的连接

    void getSize() {
        std::cout << "this->min_size = " << this->min_size << std::endl;
        std::cout << "this->max_size = " << this->max_size << std::endl;
    }

private:
    ConnectionPool();

    bool parseJsonFile();       // 解析 Json 文件
    void produceConnection();   // 生产者 -- 线程：连接不够用时，生产连接
    void recycleConnection();   // 线程：连接过剩时，销毁连接
    void addConection();        // 创建一个连接并加入连接池


private:
    // 连接数据库的信息 
    std::string ip;
    std::string user;
    std::string password;
    std::string db_name;
    unsigned short port;

    int min_size;       // 连接队列最小长度
    int max_size;       // 连接队列最大长度
    int timeout;        // 超时时间 -- 若有现成请求连接但当前连接池中没有连接，等待一个timeout时间，若有，则进行连接，若没有，则特殊处理
    int max_idle_time;  // 最大空闲时长 -- 若当前连接一次连续空闲时间超过了 max_idle_time，则认为连接池中连接数量过剩，删除该链接
    std::mutex mutex;   // 保护 connection_queue，防止多线程混乱
    std::condition_variable cond;   // 我们会定义生产者线程(创建连接)和消费者线程(使用连接)，特定条件下阻塞两个线程
    std::queue<MySQLConnect *> connection_queue;    // 存放连接的队列
};

#endif 