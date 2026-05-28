#pragma once
#include <functional>


class Thread{
public:
    using ThreadFunc = std::function<void(int)>;
    Thread();
    ~Thread();
    //开启线程
    void start();
    //获取线程id
    int getThreadId()const;

private:
    int _threadId;
    static int _generateId;
    //绑定线程函数
    ThreadFunc _threadFunc;

};