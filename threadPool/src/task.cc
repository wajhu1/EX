#include "task.hpp"
#include "result.hpp"
Task::Task(Result *result): _result(result){

}
void Task::exec(){
    // 任务执行完成后，将 run() 的返回值写入绑定的 Result。
    if(_result != nullptr){
        _result->setval(run());
    }
}
