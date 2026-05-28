#pragma once
#include <functional>

class Thread{
public:
    // 线程函数接收一个线程池内部分配的逻辑线程 id。
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc func);
    ~Thread();

    // 启动底层 std::thread。
    void start();

    // 返回线程池内部使用的逻辑线程 id。
    int getThreadId()const;


private:
    // 工作线程入口函数。
    ThreadFunc _threadFunc;
    // 逻辑线程 id，不是 std::thread::id。
    int _threadId;
    // 用于给每个 Thread 对象分配递增 id。
    static int generateId_;
};
