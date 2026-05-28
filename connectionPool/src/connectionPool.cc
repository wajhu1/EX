#include "connectionPool.hpp"
#include "log.h"


ConnectionPool::ConnectionPool(){
    if(!loadConfigFile()){
        return;
    }

    for(int i = 0; i < _initSize; ++i){
        auto p = std::make_unique<Connection> ();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshAliveTime();
        _connectionQue.push(std::move(p));
        _connectionCount++;
    }

    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();

}

ConnectionPool& ConnectionPool::getInstance(){
    static ConnectionPool instance;
    return instance;
}

std::shared_ptr<Connection> ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(_queueMtx);
    while(_connectionQue.empty()){
        if(std::cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeOut))){
            if(_connectionQue.empty()){
                log("connection timeout ... get connect error");
                return nullptr;
            }
        }
    }
    auto p = std::move(_connectionQue.front());
    _connectionQue.pop();

    std::shared_ptr<Connection> sp(p.release(), [this](Connection*conn){
        std::unique_ptr<Connection> up(conn);
        std::unique_lock<std::mutex> lock(_queueMtx);
        up->refreshAliveTime();
        _connectionQue.push(std::move(up));
    });

    if(_connectionQue.empty()){
        _cv.notify_all();
    }
    return sp;

}

void ConnectionPool::produceConnectionTask(){
    while(1){
        std::unique_lock<std::mutex> lock(_queueMtx);
        if(!_connectionQue.empty()){
            _cv.wait(lock);
        }
        for(int i = _initSize; i < _maxSize; ++i){
            auto p = std::make_unique<Connection>();
            p->refreshAliveTime();
            p->connect(_ip, _port, _username, _password, _dbname);
            _connectionQue.push(std::move(p));
            _connectionCount++;
        }

        _cv.notify_all();
    }
}

void ConnectionPool::scannerConnectionTask(){
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
        std::unique_lock<std::mutex> lock(_queueMtx);
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
        int index = str.find("=", 0);
        if(index == -1)
            continue;
        
        int endidx = str.find("\n",index);
        std::string key = str.substr(0, index);
        std::string value = str.substr(index + 1, endidx - index - 1);

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