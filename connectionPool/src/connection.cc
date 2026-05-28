#include "connection.hpp"
#include "log.h"

Connection::Connection(){
    // 初始化 MySQL 连接句柄。
    mysql_init(_conn);
}
Connection::~Connection(){
    // 释放 MySQL 连接资源。
    if(_conn){
        mysql_close(_conn);
    }
}

bool Connection::connect(std::string ip, unsigned short port, std::string username, std::string password,
                std::string dbname){
                    // 调用 MySQL C API 建立真实数据库连接。
                    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
                    return p != nullptr;
                }


bool Connection::update(std::string sql){
    // mysql_query 返回非 0 表示执行失败。
    if(mysql_query(_conn, sql.c_str())){
        log("update failed: " + sql);
        return false;
    }
    return true;
}
MYSQL_RES* Connection::query(std::string sql){
    // 查询失败时记录 SQL，成功后返回结果集。
    if(mysql_query(_conn, sql.c_str())){
        log("query failed: " + sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}
