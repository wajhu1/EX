#pragma once
#include "semaphore.hpp"
#include "any.hpp"
class Task;
class Result{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;
    //函数执行完后设置返回值
    void setVal(Any any);

    Any get();
private:
    //用于等待函数返回的指
    Semaphore _sem;
    //用于存储返回类型
    Any _any;
    //绑定任务，防止任务提前析构
    std::shared_ptr<Task> _task;
    //用于判断结果是否可用
    std::atomic_bool _isValid;
};