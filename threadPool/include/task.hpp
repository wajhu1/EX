#pragma once
#include "any.hpp"
class Result;
class Task{
public:
    // 子类需要重写 run()，在线程池工作线程中执行实际任务逻辑。
    virtual Any run() = 0;

    // Result 指针用于任务执行完成后回填结果。
    Task(Result *result = nullptr);
    ~Task() = default;

    // 由线程池调用：执行 run()，并把返回值写入 Result。
    void exec();

    // submitTask 创建 Result 后，会把 Result 地址设置到任务对象中。
    void setResult(Result *res){_result = res;}
private:
    // 不拥有 Result，只用于回写任务结果。
    Result *_result;

};
