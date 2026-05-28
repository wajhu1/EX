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
    // 获取连接池单例对象。
    static ConnectionPool& getInstance();

    //获取连接
    // 从连接池取出一个连接；调用者释放 shared_ptr 时连接会自动归还连接池。
    std::shared_ptr<Connection> getConnection();

    //增加连接
    // 生产者线程入口：当连接队列为空时创建新连接。
    void produceConnectionTask();

    //扫描连接池，超时删除
    // 扫描线程入口：定期释放超过最大空闲时间的连接。
    void scannerConnectionTask();



private:
    //初始化家在配置文件和进程
    // 构造函数私有化，保证外部只能通过 getInstance() 获取单例。
    ConnectionPool();
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool(ConnectionPool&&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ConnectionPool& operator=(ConnectionPool&&) = delete;

    // 读取 config.conf，初始化数据库连接和连接池参数。
    bool loadConfigFile();
    //读取加载文件所需的变量
    // 数据库连接参数。
    std::string _ip;
    unsigned short _port;
    std::string _username;
    std::string _dbname;
    std::string _password;
    // 初始连接数。
    size_t _initSize;
    // 最大连接数。
    size_t _maxSize;
    // 连接最大空闲时间。
    size_t _maxIdleTime;
    // 获取连接的最大等待时间。
    size_t _connectionTimeOut;

    // 空闲连接队列，队列中的连接由连接池独占管理。
    std::queue<std::unique_ptr<Connection>> _connectionQue;
    // 当前连接总数，包括空闲连接和正在被业务线程使用的连接。
    std::atomic_int _connectionCount;
    // 保护连接队列。
    std::mutex _queueMtx;
    // 用于等待可用连接或通知生产线程扩容。
    std::condition_variable _cv;


};
