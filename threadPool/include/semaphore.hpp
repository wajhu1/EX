#pragma once 
#include <condition_variable>
#include <mutex>
class Semaphore{
public:
    // num 表示初始资源计数，默认为 0，表示 wait() 会阻塞。
    Semaphore(int num = 0);
    ~Semaphore();

    // P 操作：资源计数为 0 时阻塞，直到 post() 增加计数。
    void wait();

    // V 操作：增加资源计数，并唤醒等待线程。
    void post();

private:
    // 保护 _num 和条件变量等待过程。
    std::mutex _mutex;
    std::condition_variable _cv;
    // 当前可用资源数量。
    int _num;
    // 标记信号量对象是否正在析构。
    std::atomic_bool _isExit;

};
