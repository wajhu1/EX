#pragma once
#include <memory>
#include <atomic>
#include "semaphore.hpp"
#include "any.hpp"
#include "task.hpp"

class Result{
public:

    // Result 和 Task 绑定：任务执行结束后会通过 Result::setval 写回结果。
    // isValid 用于表示任务是否成功提交到线程池。
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result();

    // 由工作线程调用，保存任务返回值并唤醒等待 get() 的线程。
    void setval(Any any);

    // 由提交任务的线程调用；如果结果还没准备好，会阻塞等待。
    Any get();
private:
    // 保存任务返回值，Any 负责擦除具体返回类型。
    Any _any;
    // 用于实现“结果未就绪则等待，结果就绪则唤醒”的同步。
    Semaphore _sem;
    // 标记 Result 是否有效，例如任务队列满导致提交失败时为 false。
    std::atomic_bool _isValid;
    // 持有任务对象，避免任务在执行完成前被提前释放。
    std::shared_ptr<Task> _task;

};
