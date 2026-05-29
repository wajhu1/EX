#pragma once
#include "result.hpp"
class Any;
class Task{
public:
    Task(Result* res = nullptr);
    virtual Any run() = 0;
    void exec();
    void setResult(Result *res);
private:
    //绑定结果，用于设置结果的值
    Result *_result;
};