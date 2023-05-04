#ifndef LAB_MYSQL_CONNECT_MYSQL_CONNECT_H__
#define LAB_MYSQL_CONNECT_MYSQL_CONNECT_H__

//===------- mysql_connect/mysql_connect.h - class definition ---*- C++ -*-===//
//
/// 初始化数据库连接
/// 释放数据库连接
/// 连接数据库
/// 更新数据库：insert,  update,  delete
/// 查询数据库
/// 遍历查询得到的结果集
/// 得到结果集中的字段值
/// 事务操作
/// 提交事务
/// 回滚事务
//
//===----------------------------------------------------------------------===//

#include <chrono>
#include <iostream>
#include <string>
#include <mysql/mysql.h>

class MySQLConnect
{
public:
    // 初始化数据库连接
    MySQLConnect();

    // 释放数据库连接
    ~MySQLConnect();

    // 连接数据库
    bool connect(std::string user, std::string passwd, std::string dbName, std::string ip, unsigned short port = 3306);

    // 更新数据库
    bool update(std::string sql);

    // 查询数据库
    bool query(std::string sql);

    // 遍历查询得到的结果集, 每调用一次 next(), 返回结果集中的一个新的记录
    bool next();

    // 得到结果集中的字段值
    std::string getValue(int index);

    // 事务操作, 创建事务 -- 将事务设置成手动提交，调用该函数后，数据库会为我们创建一个保存点
    bool transaction();

    // 提交事务
    bool commit();

    // 事务回滚
    bool rollback();

    // 刷新起始的空闲时间点
    void refreshAliveTime();

    // 计算连接存活的总时长
    long long getAliveTime();

private:
    void freeResult();

private:
    MYSQL * m_connect{0};           // 这部分地址在 析构函数 中释放
    MYSQL_RES * m_result{0};        // 这块地址要手动释放
    MYSQL_ROW m_row{0};
    std::chrono::steady_clock::time_point alive_time;
};

#endif