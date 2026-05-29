#include "task.hpp"


Task::Task(Result* res):_result(res){

}
void Task::exec(){
    if(_result != nullptr){
        return _result->setVal(run());
    }

}
void Task::setResult(Result *res){
    _result = res;
}