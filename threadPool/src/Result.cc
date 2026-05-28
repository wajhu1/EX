#include "result.hpp"


Result::Result(std::shared_ptr<Task> task, bool isValid ):_task(task), _isValid(isValid){
    // 让任务持有当前 Result 地址，执行完成后可以回写返回值。
    _task->setResult(this);
}
Result::~Result(){

}


void Result::setval(Any any){
    // 保存任务返回值，并通过信号量通知 get() 可以返回。
    _any = std::move(any);
    _sem.post();
}

Any Result::get(){
    // 无效 Result 通常表示任务提交失败，直接返回空 Any。
    if(!_isValid){
        return Any();
    }
    // 等待工作线程执行完任务并调用 setval()。
    _sem.wait();
    return std::move(_any);
}
