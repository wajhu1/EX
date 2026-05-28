#include "thread.hpp"
#include <thread>

Thread::Thread(ThreadFunc func): _threadId(generateId_++){
    // 保存线程入口函数，start() 时再真正创建系统线程。
    _threadFunc = func;
}
Thread::~Thread(){

}

void Thread::start(){
    // 创建底层线程并传入逻辑线程 id。
    std::thread t(_threadFunc, _threadId);
    // 当前实现采用分离线程，由线程池内部状态协调退出。
    t.detach();
}

int Thread::generateId_ = 0;

int Thread::getThreadId()const{
    return _threadId;
}
