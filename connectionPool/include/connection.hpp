#pragma once

#include <mysql.h>
#include <ctime>
#include <iostream>

class Connection{
public:
    Connection();
    ~Connection();

    // 建立到 MySQL 服务器的连接。
    bool connect(std::string ip, unsigned short port, std::string username, std::string password,
                std::string dbname);

    // 执行 insert、delete、update 等不返回结果集的 SQL。
    bool update(std::string sql);
    // 执行 select 查询，返回 MySQL 原生结果集指针。
    MYSQL_RES* query(std::string sql);

    // 返回连接从上次刷新开始到现在的存活时间。
    clock_t getAliveTime(){return clock() - _aliveTime;}
    // 刷新连接的起始存活时间，通常在连接归还到池中时调用。
    void refreshAliveTime(){_aliveTime = clock();}

private:
    // MySQL C API 的连接句柄。
    MYSQL *_conn;
    // 记录连接进入空闲状态的时间点，用于扫描线程判断是否超时。
    clock_t _aliveTime;
};
