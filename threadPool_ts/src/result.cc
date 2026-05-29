#include "result.hpp"
#include "task.hpp"

Result::Result(std::shared_ptr<Task> task, bool isValid):_task(task), _isValid(isValid),_sem(0){
    if(_isValid)
        _task->setResult(this);
}

//函数执行完后设置返回值
void Result::setVal(Any any){
    _any = std::move(any);

    _sem.post();
}

Any Result::get(){
    if(!_isValid){
        return Any();
    }
    _sem.wait();
    return std::move(_any);
}