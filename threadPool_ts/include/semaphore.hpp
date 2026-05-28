#pragma once
#include <mutex>
#include <condition_variable>

class Semaphore{
public:
    Semaphore();
    ~Semaphore();
    
    void wait();


    void post();


private:
    
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic_int _num;
    std::atomic_bool _isExit;

};