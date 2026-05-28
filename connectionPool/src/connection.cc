#include "connection.hpp"
#include "log.h"

Connection::Connection(){
    mysql_init(_conn);
}
Connection::~Connection(){
    if(_conn){
        mysql_close(_conn);
    }
}

bool Connection::connect(std::string ip, unsigned short port, std::string username, std::string password,
                std::string dbname){
                    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
                    return p != nullptr;
                }


bool Connection::update(std::string sql){
    if(mysql_query(_conn, sql.c_str())){
        log("update failed: " + sql);
        return false;
    }
    return true;
}
MYSQL_RES* Connection::query(std::string sql){
    if(mysql_query(_conn, sql.c_str())){
        log("query failed: " + sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}