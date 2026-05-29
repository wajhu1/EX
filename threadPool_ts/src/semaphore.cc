#include "semaphore.hpp"

Semaphore::Semaphore(int num):_num(num), _isExit(false){

}
Semaphore::~Semaphore(){
    _isExit = true;
    _cv.notify_all();
}

void Semaphore::wait(){
    if(_isExit)
        return;
    
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [&](){
        return _num > 0 || _isExit;
    });
    _num--;
}


void Semaphore::post(){
    if(_isExit)
        return;

    std::unique_lock<std::mutex> lock(_mutex);
    _num++;
    _cv.notify_all();
}