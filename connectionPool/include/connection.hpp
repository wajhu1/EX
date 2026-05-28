#pragma once

#include <mysql.h>
#include <ctime>
#include <iostream>

class Connection{
public:
    Connection();
    ~Connection();

    bool connect(std::string ip, unsigned short port, std::string username, std::string password,
                std::string dbname);

    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);

    clock_t getAliveTime(){return clock() - _aliveTime;}
    void refreshAliveTime(){_aliveTime = clock();}

private:
    MYSQL *_conn;
    clock_t _aliveTime;
};