#pragma once 
#include <unordered_map>
#include "thread.hpp"
#include <memory>
#include "semaphore.hpp"
#include "task.hpp"
#include "result.hpp"
#include <queue>
#include <thread>


//线程模型
enum PoolMode{
    MODE_FIXED,
    MODE_CACHED,
};

class ThreadPool{
public:
    ThreadPool();
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void setPoolMode(PoolMode mode);

    void setThreadThreshHold(size_t num);

    void setTaskThreshHold(size_t num);

    Result submitTask(std::shared_ptr<Task> task);

    void threadFunc(size_t threadId);

    void start(int num = std::thread::hardware_concurrency());



private:
    bool checkPoolRunning();

    std::unordered_map<size_t, std::unique_ptr<Thread>> _threads;
    std::atomic_int _curThreadSize;
    std::atomic_int _idleThreadSize;
    size_t _initThreadSize;
    size_t _threadThreshHold;
    
    std::queue<std::shared_ptr<Task>> _taskQue;
    size_t _taskThrshHold;

    std::mutex _taskQueMtx;
    std::condition_variable _notFull;
    std::condition_variable _notEmpty;

    PoolMode _poolMode;

    std::atomic_bool _isPoolRunning;
    std::condition_variable _isExit;
};