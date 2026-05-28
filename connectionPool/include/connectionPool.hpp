#pragma once
#include <iostream>
#include <queue>
#include <memory.h>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <functional>

#include "connection.hpp"

class ConnectionPool{
    static ConnectionPool& getInstance();

    //获取连接
    std::shared_ptr<Connection> getConnection();

    //增加连接
    void produceConnectionTask();

    //扫描连接池，超时删除
    void scannerConnectionTask();



private:
    //初始化家在配置文件和进程
    ConnectionPool();
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool(ConnectionPool&&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ConnectionPool& operator=(ConnectionPool&&) = delete;

    bool loadConfigFile();
    //读取加载文件所需的变量
    std::string _ip;
    unsigned short _port;
    std::string _username;
    std::string _dbname;
    std::string _password;
    size_t _initSize;
    size_t _maxSize;
    size_t _maxIdleTime;
    size_t _connectionTimeOut;

    std::queue<std::unique_ptr<Connection>> _connectionQue;
    std::atomic_int _connectionCount;
    std::mutex _queueMtx;
    std::condition_variable _cv;


};