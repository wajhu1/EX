#include "semaphore.hpp"

Semaphore::Semaphore(int num):_num(num), _isExit(false){

}

Semaphore::~Semaphore(){
    // 标记对象正在退出。
    _isExit = true;
}

void Semaphore::wait(){
    // 对象已退出时直接返回。
    if(_isExit){
        return;
    }
    std::unique_lock<std::mutex> lock(_mutex);

    // 等待资源计数变为正数。
    _cv.wait(lock, [&](){
        return _num > 0;
    });

    // 消耗一个资源计数。
    _num--;    
}

void Semaphore::post(){
    // 对象已退出时不再投递信号。
    if(_isExit)
        return;
    std::unique_lock<std::mutex> lock(_mutex);
    // 增加资源计数，并唤醒等待线程。
    _num++;
    _cv.notify_all();
}
