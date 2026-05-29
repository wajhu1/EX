#pragma once
#include <mutex>
#include <condition_variable>

class Semaphore{
public:
    Semaphore(int num = 0);
    ~Semaphore();
    
    void wait();


    void post();


private:
    
    std::mutex _mutex;
    std::condition_variable _cv;
    int _num;
    std::atomic_bool _isExit;

};