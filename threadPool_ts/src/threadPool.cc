#include "threadpool.hpp"
#include <chrono>
#include "public.hpp"

ThreadPool::ThreadPool():
    _threadThreshHold(THREAD_THRESH_HOLD),
    _initThreadSize(0),
    _idleThreadSize(0),
    _curThreadSize(0),
    _taskThrshHold(TASK_THRESH_HOLD),
    _poolMode(PoolMode::MODE_FIXED),
    _isPoolRunning(false){

}
ThreadPool::~ThreadPool(){
    _isPoolRunning = false;

    std::unique_lock<std::mutex> lock(_taskQueMtx);
    _notEmpty.notify_all();
    _isExit.wait(lock, [&](){
        return _threads.size() == 0;
    });
}



void ThreadPool::setPoolMode(PoolMode mode){
    if(checkPoolRunning())
        return;
    _poolMode = mode;
}

void ThreadPool::setThreadThreshHold(size_t num){
    if(checkPoolRunning())
        return;

    _threadThreshHold = num;
}

void ThreadPool::setTaskThreshHold(size_t num){
    if(checkPoolRunning())
        return;

    _taskThrshHold = num;
}

Result ThreadPool::submitTask(std::shared_ptr<Task> task){
    std::unique_lock<std::mutex> lock(_taskQueMtx);


    if(!_notFull.wait_for(lock, std::chrono::seconds(1), [&](){
        return _taskQue.size() < _taskThrshHold;
    })){
        return Result(task, false);
    }
    _taskQue.emplace(task);

    _notEmpty.notify_all();

    if(_poolMode == PoolMode::MODE_CACHED &&
        _taskQue.size() > _idleThreadSize &&
        _curThreadSize < _threadThreshHold){

        auto sp = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        size_t threadId = sp->getThreadId();
        _threads.emplace(threadId, std::move(sp));
        _threads[threadId]->start();
        _curThreadSize++;
        _idleThreadSize++;
    }


    return Result(task);

}

void ThreadPool::threadFunc(size_t threadId){
    auto lastTime = std::chrono::high_resolution_clock::now();
    for(;;){
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(_taskQueMtx);
            while(_taskQue.size() == 0){
                if(!_isPoolRunning){
                    _threads.erase(threadId);
                    _isExit.notify_all();
                    return;
                }

                if(_poolMode == PoolMode::MODE_CACHED){
                    if(std::cv_status::timeout == _notEmpty.wait_for(lock, std::chrono::seconds(1))){
                        auto now = std::chrono::high_resolution_clock::now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if(dur.count() > MAX_IDLE_TIME && _curThreadSize > _initThreadSize){
                            _threads.erase(threadId);
                            _curThreadSize--;
                            _idleThreadSize--;
                            _isExit.notify_all();
                            return;
                        }

                    }
                }else{
                    _notEmpty.wait(lock,[&](){
                        return !_taskQue.empty() || !_isPoolRunning;
                    });
                }


            }

            task = _taskQue.front();
            _taskQue.pop();
            _idleThreadSize--;
            if(_taskQue.size() != 0)
                _notEmpty.notify_all();
            _notFull.notify_all();

        }

        if(task != nullptr){
            task->exec();
        }

        _idleThreadSize++;
        lastTime = std::chrono::high_resolution_clock::now();
    }
}

void ThreadPool::start(int num){
    _isPoolRunning = true;
    _curThreadSize = num;
    _initThreadSize = num;

    for(int i = 0; i < _initThreadSize; ++i){
        auto sp = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        size_t threadId = sp->getThreadId();
        _threads.emplace(threadId, std::move(sp));
    }

    for(auto &thread : _threads){
        thread.second->start();
        _idleThreadSize++;      
    }

}

bool ThreadPool::checkPoolRunning(){
    return _isPoolRunning;
}
