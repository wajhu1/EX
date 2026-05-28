#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include "result.hpp"
#include <unordered_map>
#include "thread.hpp"
#include <chrono>
#include <iostream>
#include "task.hpp"
class Thread;

class Task;

enum PoolMode{
    // 固定线程数模式：启动后线程数保持为 initThreadSize。
    MODE_FIXED,
    // 缓存模式：任务多时可扩容线程，空闲过久后回收线程。
    MODE_CACHED,
};



class ThreadPool{
public:
    ThreadPool();
    ~ThreadPool();

    // 启动线程池，创建 initThreadSize 个初始工作线程。
    void start(int initThreadSize = std::thread::hardware_concurrency());

    // 设置线程池模式；必须在线程池启动前调用。
    void setMode(PoolMode mode);

    // 设置任务队列容量上限；必须在线程池启动前调用。
    void setTaskQueThreshHold(int threshhold);

    // 设置 cached 模式下线程数量上限；必须在线程池启动前调用。
    void setThreadSizeThreshHold(int threshhold);

    // 提交任务到队列，返回用于获取异步结果的 Result。
    Result submitTask(std::shared_ptr<Task> sp);

    ThreadPool(const ThreadPool&) = delete;

    // 工作线程主循环：等待任务、取任务、执行任务、处理退出。
    void threadFunc(int threadId);

    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    // 判断线程池是否已经启动。
    bool checkRunningState()const;

    // std::vector<std::unique_ptr<Thread>> _threads;
    // 保存当前存活的工作线程对象，key 是逻辑线程 id。
    std::unordered_map<int, std::unique_ptr<Thread>> _threads;
    // 当前空闲线程数量。
    std::atomic_int _idleThreadSize;
    // 初始线程数量。
    size_t _initThreadSize;
    // cached 模式下允许创建的最大线程数量。
    size_t _threadSizeThreshHold;
    // 当前线程总数量。
    std::atomic_int _curThreadSize;

    // 任务队列，保存等待执行的任务。
    std::queue<std::shared_ptr<Task>> _taskQue;
    // 当前任务数量。
    std::atomic_uint _taskSize;
    // 任务队列容量上限。
    size_t _taskQueThreshHold;

    // 保护任务队列及相关条件变量。
    std::mutex _taskQueMtx;
    // 队列未满条件：submitTask 在队列满时等待。
    std::condition_variable _notFull;
    // 队列非空条件：工作线程在无任务时等待。
    std::condition_variable _notEmpty;

    // 当前线程池模式。
    PoolMode _poolMode;

    // 线程池运行状态。
    std::atomic_bool _isPoolRunning;
    // 析构时等待所有工作线程退出。
    std::condition_variable _exitCond;
};
