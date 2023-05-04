#include "./mysql_connect.h"
#include <iostream>

/**
 * @brief 初始化 MySQL 连接并将字符集设置为“utf8”。
 */
MySQLConnect::MySQLConnect()
{
    this->m_connect = mysql_init(nullptr);
    mysql_set_character_set(this->m_connect, "utf8");
}

/**
 * @brief 如果 MySQLConnect 类的 m_connect 不为空，则关闭与 MySQL 数据库的连接。
 */
MySQLConnect::~MySQLConnect()
{
    if (this->m_connect != nullptr) {
        mysql_close(this->m_connect);
    }
    this->freeResult();
}

/**
 * @brief 该函数使用提供的用户凭证、数据库名称、IP 地址和端口号连接到 MySQL 数据库。
 * 
 * @param user 用于连接到 MySQL 数据库的用户名。
 * @param passwd 用于连接到数据库的 MySQL 用户帐户的密码。
 * @param dbName dbName 代表“数据库名称”，指的是您要连接的 MySQL 数据库的名称。
 * @param ip 要连接的 MySQL 服务器的 IP 地址。
 * @param port port 参数是一个无符号短整数，它指定用于 MySQL 连接的端口号。 MySQL 的默认端口号是 3306。
 * 
 * @return 函数 connect 返回一个布尔值，指示与 MySQL 数据库的连接是否成功。如果连接成功则返回“true”，否则返回“false”。
 */
bool MySQLConnect::connect(std::string user, std::string passwd, std::string dbName, std::string ip, unsigned short port)
{
    MYSQL * ret = mysql_real_connect(this->m_connect, ip.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, nullptr, 0);
    return ret != nullptr;
}

/**
 * @brief 该函数执行 SQL 更新查询并返回一个布尔值，指示查询是否成功。
 * 
 * @param sql 参数“sql”是一个字符串，其中包含要由 MySQL 数据库执行的 SQL 查询。 “更新”函数将此 SQL 查询作为输入，使用 MySQL
 *            连接对象“m_connect”执行它，并返回一个布尔值，指示查询是否已成功执行。
 * 
 * @return 函数“update”返回一个布尔值。如果`mysql_query`函数返回0，则返回`true`，表示SQL查询执行成功。否则，它返回“false”。
 */
bool MySQLConnect::update(std::string sql)
{
    int ret = mysql_query(this->m_connect, sql.c_str());
    return ret == 0;
}

/**
 * @brief 此函数执行 MySQL 查询，并将结果集存放到 this->m_result 中
 * 
 * @param sql 包含要执行的 SQL 查询的字符串。
 * 
 * @return 该函数返回一个布尔值。如果查询执行成功并存储结果，则返回 true；如果执行查询或存储结果出错，则返回 false。
 */
bool MySQLConnect::query(std::string sql) 
{
    this->freeResult();

    int ret = mysql_query(this->m_connect, sql.c_str());
    if (ret != 0) {
        return false;
    }
    this->m_result = mysql_store_result(this->m_connect);
    if (this->m_result == nullptr) {
        return false;
    }
    return true;
}

/**
 * 如果 MySQL 结果集中有下一行，则该函数返回 true，否则返回 false。
 * 
 * @return 该函数返回一个布尔值。如果要获取的结果集中有另一行可用，则返回 true；如果没有更多要获取的行或结果集为空，则返回 false。
 */
bool MySQLConnect::next()
{
    if (this->m_result != nullptr) {
        this->m_row = mysql_fetch_row(this->m_result);
        if (this->m_row != nullptr) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 该函数根据给定的索引从 MySQL 数据库结果集中检索一个索引
 * 
 * @param index index参数是一个整数值，表示MySQL查询结果中需要取值的列的索引。
 * 
 * @return 此函数从 MySQL 查询结果集中的特定列索引返回字符串值。如果索引超出范围或为负数，则返回空字符串。
 *         该函数从结果集的当前行中检索值并将其从 char 指针转换为字符串
 *         因为 char * 转换到 std::string 时'\0' 被抛弃，所以还要考虑值的长度。
 */
std::string MySQLConnect::getValue(int index)
{
    int columns_count = mysql_num_fields(this->m_result);
    if (index >= columns_count || index < 0) {
        return std::string();
    }
    char * value = this->m_row[index];
    unsigned long length = mysql_fetch_lengths(this->m_result)[index];      // 获得目标字段的长度
    return std::string(value, length);      // 这里从 char * 转换为 std::string 时，char * 的结束字符 '\0' 会被抛弃
}

/**
 * @brief 该函数将 MySQL 连接的自动提交模式设置为 false(即手动提交)，启用事务。
 * 
 * @return 函数返回一个布尔值。它返回被设置为手动提交的“mysql_autocommit()”函数的结果。 
 *         `mysql_autocommit()` 函数用于关闭 MySQL 事务的自动提交模式。如果函数调用成功，则返回“true”，否则返回“false”。
 */
bool MySQLConnect::transaction()
{
    return mysql_autocommit(this->m_connect, false);
}

/**
 * @brief 该函数在 MySQL 中提交事务并返回一个布尔值，指示操作是否成功。
 * 
 * @return 返回一个布尔值。它返回 `mysql_commit()` 函数的结果，这是一个布尔值，指示提交操作是否成功。
 */
bool MySQLConnect::commit()
{
    return mysql_commit(this->m_connect);
}


/**
 * @brief 该函数对 MySQL 数据库连接执行回滚操作。
 * 
 * @return `rollback()` 函数返回一个布尔值。它返回 mysql_rollback() 函数的结果，这是一个布尔值，指示回滚操作是否成功。
 */
bool MySQLConnect::rollback()
{
    return mysql_rollback(this->m_connect);
}

/**
 * @brief 该函数释放为 MySQL 结果集分配的内存。
 */
void MySQLConnect::freeResult()
{
    if (this->m_result != nullptr) {
        mysql_free_result(this->m_result);
        this->m_result = nullptr;
    }
}

/**
 * @brief 将该链接开始空闲的时间点保存到 this->alive_time 中
 */
void MySQLConnect::refreshAliveTime()
{
    this->alive_time = std::chrono::steady_clock::now();    
}

/**
 * @brief 用于查看空闲时间
 * 
 * @return 自 MySQL 连接建立以来 或 上次被调用后归还后 经过的时间（以毫秒为单位）。
 */
long long MySQLConnect::getAliveTime()
{
    std::chrono::nanoseconds result = std::chrono::steady_clock::now() - this->alive_time;
    std::chrono::milliseconds millisec = std::chrono::duration_cast<std::chrono::milliseconds>(result); // 从高精度向低精度转换需要使用 duration_cast<>
    return millisec.count();
}