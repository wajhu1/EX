#include "thread.hpp"
#include <thread>

Thread::Thread(ThreadFunc threadFunc):_threadFunc(threadFunc),_threadId(_generateId++){

}
Thread::~Thread(){

}
//开启线程
void Thread::start(){
    std::thread t(_threadFunc, _threadId);
    t.detach();
}
//获取线程id
size_t Thread::getThreadId()const{
    return _threadId;
}

size_t Thread::_generateId = 0;