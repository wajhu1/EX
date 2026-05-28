
#include "threadpool.hpp"
const int MAXTHRESH = 1024;
const int THREAD_MAX_THRESHHOLD = 10;
const int THREAD_MAX_IDLE_TIME = 60;

ThreadPool::ThreadPool():_initThreadSize(0), 
    _taskQueThreshHold(MAXTHRESH), 
    _isPoolRunning(false),
    _idleThreadSize(0),
    _threadSizeThreshHold(THREAD_MAX_THRESHHOLD),
    _curThreadSize(0),
    _poolMode(PoolMode::MODE_FIXED),
    _taskSize(0){

}
ThreadPool::~ThreadPool(){
    // 通知所有工作线程：线程池即将停止。
    _isPoolRunning = false;

    std::unique_lock<std::mutex> lock(_taskQueMtx);
    // 唤醒等待任务的工作线程，让它们有机会检查退出状态。
    _notEmpty.notify_all();
    // 等待所有工作线程从 _threads 中移除后再析构线程池。
    _exitCond.wait(lock, [&](){
        return _threads.size() == 0;
    });
}

void ThreadPool::start(int initThreadSize){
    // 标记线程池已启动，并记录初始线程数量。
    _isPoolRunning = true;
    _initThreadSize = initThreadSize;
    _curThreadSize = _initThreadSize;

    // 先创建 Thread 对象并放入容器，稍后统一启动。
    for(int i = 0; i < _initThreadSize; i++){
        // auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        // _threads.emplace_back(std::move(ptr));
        //_threads.emplace_back(std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this)));
        // _threads.emplace_back(std::make_unique<Thread>([this](){
        //     this->threadFunc();
        // }));
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this, std::placeholders::_1));
        int threadId = ptr->getThreadId();
        _threads.emplace(threadId, std::move(ptr));
    }

    // 启动所有初始线程，启动后它们会进入 threadFunc 等待任务。
    for(auto &item : _threads){
        item.second->start();
        _idleThreadSize++;
    }
}

void ThreadPool::threadFunc(int threadId){
    // cached 模式下用于计算线程空闲时间。
    auto lastTime = std::chrono::high_resolution_clock::now();
    for(;;){
        std::shared_ptr<Task> t;
        {
            std::unique_lock<std::mutex> lock(_taskQueMtx);

            // 没有任务时，工作线程进入等待或在 cached 模式下超时检查。
            while(_taskQue.size() == 0){
                if(!_isPoolRunning){
                    // 线程池停止时，当前线程从线程表中移除并退出。
                    _threads.erase(threadId);
                    std::cout<<"threadId:"<<std::this_thread::get_id()<<"exit"<<std::endl;
                    _exitCond.notify_all();
                    return;
                }
                if(_poolMode == PoolMode::MODE_CACHED){
                    // cached 模式下不会无限等待，每秒醒来检查一次空闲时长。
                    if(std::cv_status::timeout == 
                        _notEmpty.wait_for(lock, std::chrono::seconds(1))){
                            auto now = std::chrono::high_resolution_clock::now();
                            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                            if(dur.count() >= THREAD_MAX_IDLE_TIME){
                                // 空闲时间超过阈值后回收当前线程。
                                _threads.erase(threadId);
                                _curThreadSize--;
                                _idleThreadSize--;
                                std::cout<<"threadId:"<<std::this_thread::get_id()<<"exit"<<std::endl;
                                _exitCond.notify_all();
                                return;
                            }
                        }
                }else{
                        // fixed 模式下，线程持续等待新任务到来。
                        _notEmpty.wait(lock,[&](){
                            return !_taskQue.empty();
                        });
                }
                // if(!_isPoolRunning){
                //     _threads.erase(threadId);
                //     _curThreadSize--;
                //     _idleThreadSize--;
                //     std::cout<<"threadId:"<<std::this_thread::get_id()<<"exit"<<std::endl;
                //     _exitCond.notify_all();
                //     return;
                // }
            }

            // 当前线程即将取走一个任务，因此从空闲线程数中扣除。
            _idleThreadSize--;

            t = _taskQue.front();
            
            _taskQue.pop();
            _taskSize--;
            if(_taskQue.size() > 0){
                // 如果队列里还有任务，继续唤醒其他空闲线程处理。
                _notEmpty.notify_all();
            }
            // 队列弹出一个任务后，通知 submitTask 队列可能已不满。
            _notFull.notify_all();
        }
        if(t != nullptr){
            // 在锁外执行任务，避免长时间占用任务队列锁。
            t->exec();
        }
        // 任务执行完成，当前线程重新变为空闲。
        _idleThreadSize++;
        lastTime = std::chrono::high_resolution_clock::now();
    }


}

void ThreadPool::setMode(PoolMode mode){
    // 启动后不允许再切换模式。
    if(checkRunningState()){
        return ;
    }
    _poolMode = mode;
}

void ThreadPool::setTaskQueThreshHold(int threshhold){
    // 启动后不允许再修改任务队列上限。
    if(checkRunningState())
        return;
    _taskQueThreshHold = threshhold;
}

Result ThreadPool::submitTask(std::shared_ptr<Task> sp){
    std::unique_lock<std::mutex> lock(_taskQueMtx);

    // 队列满时最多等待 1 秒；超时说明提交失败，返回无效 Result。
    if(!_notFull.wait_for(lock,std::chrono::seconds(1) , [&](){return _taskQue.size() < _taskQueThreshHold;})){
        //超时未提交
        std::cerr << "task queue is full, submit task fail."<<std::endl;
        return Result(sp, false);
    }

    // 将任务放入等待队列。
    _taskQue.emplace(sp);
    _taskSize++;

    // 通知工作线程队列中已有任务。
    _notEmpty.notify_all();


    // cached 模式下，如果任务数量多于空闲线程，并且还没超过线程上限，则扩容线程。
    if(_poolMode == PoolMode::MODE_CACHED
        && _taskSize > _idleThreadSize
        && _curThreadSize < _threadSizeThreshHold){
            auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            auto threadId = ptr->getThreadId();
            _threads.emplace(threadId,std::move(ptr));
            _threads[threadId]->start();
            _curThreadSize++;
            _idleThreadSize++;
    }



    // 返回 Result，调用者可通过 get() 等待任务执行结果。
    return Result(sp);
}



bool ThreadPool::checkRunningState()const{
    return _isPoolRunning;
}


void ThreadPool::setThreadSizeThreshHold(int threshhold){
    // 启动后不允许再修改线程数量上限。
    if(checkRunningState())
        return;
    // 线程数量上限只对 cached 模式有意义。
    if(_poolMode == PoolMode::MODE_CACHED)
        _threadSizeThreshHold = threshhold;
}
