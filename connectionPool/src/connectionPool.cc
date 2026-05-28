#include "connectionPool.hpp"
#include "log.h"


ConnectionPool::ConnectionPool(){
    // 先加载数据库和连接池配置，加载失败则不继续初始化连接。
    if(!loadConfigFile()){
        return;
    }

    // 创建初始数量的数据库连接，并放入空闲连接队列。
    for(int i = 0; i < _initSize; ++i){
        auto p = std::make_unique<Connection> ();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshAliveTime();
        _connectionQue.push(std::move(p));
        _connectionCount++;
    }

    // 启动连接生产线程，负责在连接不足时补充连接。
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();
    // 启动扫描线程，负责回收空闲超时的连接。
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();

}

ConnectionPool& ConnectionPool::getInstance(){
    // 函数内静态变量在 C++11 之后线程安全初始化。
    static ConnectionPool instance;
    return instance;
}

std::shared_ptr<Connection> ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(_queueMtx);
    // 没有空闲连接时，等待生产线程创建连接或等待其他线程归还连接。
    while(_connectionQue.empty()){
        if(std::cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeOut))){
            if(_connectionQue.empty()){
                log("connection timeout ... get connect error");
                return nullptr;
            }
        }
    }
    // 从空闲队列中取出一个连接，所有权临时转移给调用方。
    auto p = std::move(_connectionQue.front());
    _connectionQue.pop();

    // 自定义删除器：shared_ptr 释放时不销毁连接，而是把连接归还连接池。
    std::shared_ptr<Connection> sp(p.release(), [this](Connection*conn){
        std::unique_ptr<Connection> up(conn);
        std::unique_lock<std::mutex> lock(_queueMtx);
        up->refreshAliveTime();
        _connectionQue.push(std::move(up));
    });

    // 连接被取空后，通知生产线程尝试创建更多连接。
    if(_connectionQue.empty()){
        _cv.notify_all();
    }
    return sp;

}

void ConnectionPool::produceConnectionTask(){
    while(1){
        std::unique_lock<std::mutex> lock(_queueMtx);
        // 队列不为空时生产线程休眠，等待连接被取空后再醒来。
        if(!_connectionQue.empty()){
            _cv.wait(lock);
        }
        // 连接不足时补充连接，直到达到最大连接数。
        for(int i = _initSize; i < _maxSize; ++i){
            auto p = std::make_unique<Connection>();
            p->refreshAliveTime();
            p->connect(_ip, _port, _username, _password, _dbname);
            _connectionQue.push(std::move(p));
            _connectionCount++;
        }

        // 通知等待连接的业务线程：现在可能有可用连接了。
        _cv.notify_all();
    }
}

void ConnectionPool::scannerConnectionTask(){
    while(1){
        // 每隔 maxIdleTime 秒扫描一次空闲连接。
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
        std::unique_lock<std::mutex> lock(_queueMtx);
        // 保留至少 initSize 个连接，多余连接如果空闲超时则释放。
        while(_connectionCount > _initSize){
            auto &p = _connectionQue.front();
            if(p->getAliveTime() >= (_maxIdleTime * 100)){
                _connectionQue.pop();
                _connectionCount--;
            }else{
                break;
            }
        }

    }
}

bool ConnectionPool::loadConfigFile(){
    // 从当前工作目录读取连接池配置文件。
    FILE* pf = fopen("config.conf", "r");
    if(pf == nullptr){
        log("loadConfigFile failed");
        return false;
    }
    //feof:是否读取到了文件尾
    while(!feof(pf)){
        char buf[1024] = {0};
        fgets(buf, 1024, pf);
        std::string str = buf;
        // 配置项格式为 key=value，找不到等号则跳过。
        int index = str.find("=", 0);
        if(index == -1)
            continue;
        
        int endidx = str.find("\n",index);
        std::string key = str.substr(0, index);
        std::string value = str.substr(index + 1, endidx - index - 1);

        // 根据配置 key 填充连接池参数。
        if(key == "ip"){
            _ip = value;
        }else if(key == "port"){
            _port = atoi(value.c_str());
        }else if(key == "username"){
            _username = value;
        }else if(key == "password"){
            _password = value;
        }else if(key == "dbname"){
            _dbname = value;
        }else if(key == "initSize"){
            _initSize = atoi(value.c_str());
        }else if(key == "maxSize"){
            _maxSize = atoi(value.c_str());
        }else if(key == "maxIdleTime"){
            _maxIdleTime = atoi(value.c_str());
        }else if(key == "connectionTimeOut"){
            _connectionTimeOut = atoi(value.c_str());
        }
    }
    return true;
}
