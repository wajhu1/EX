#pragma once
#include <functional>


class Thread{
public:
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc threadFunc);
    ~Thread();
    //开启线程
    void start();
    //获取线程id
    size_t getThreadId()const;

private:
    size_t _threadId;
    static size_t _generateId;
    //绑定线程函数
    ThreadFunc _threadFunc;

};