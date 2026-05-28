// #include "threadpool.hpp"

// class MyTask : public Task{
// public:
//     MyTask(int a, int b):begin(a), end(b){

//     }
//     Any run()override{
//         std::this_thread::sleep_for(std::chrono::seconds(2));
//         return begin + end;
//     }
// private:
//     int begin;
//     int end;
// };


// int main(){
//     ThreadPool threadpool;
//     threadpool.setMode(PoolMode::MODE_FIXED);
//     threadpool.start(4);

//     threadpool.submitTask(std::make_shared<MyTask>(1,1));
//     threadpool.submitTask(std::make_shared<MyTask>(1,1));
//     threadpool.submitTask(std::make_shared<MyTask>(1,1));
//     Result a = threadpool.submitTask(std::make_shared<MyTask>(1,6));
//     Result b = threadpool.submitTask(std::make_shared<MyTask>(5,6));
//     Result c = threadpool.submitTask(std::make_shared<MyTask>(6,6));
//     Result d = threadpool.submitTask(std::make_shared<MyTask>(6,6));
//     Result e = threadpool.submitTask(std::make_shared<MyTask>(6,6));
//     threadpool.submitTask(std::make_shared<MyTask>(1,1));
//     int v1 = a.get().cast_<int>();
//     int v2 = b.get().cast_<int>();
//     int v3 = c.get().cast_<int>();
//     int v4 = d.get().cast_<int>();
//     int v5 = e.get().cast_<int>();

//     int sum = v1 + v2 + v3 + v4 + v5;
//     std::cout<<v1<<" "<<v2<<" "<<v3<<" "<<v4 << " "<<v5 << " "<<sum<<std::endl;
// }

#include "threadpool.hpp"

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

class SlowTask : public Task {
public:
    SlowTask(int a, int b, int seconds)
        : begin(a), end(b), seconds(seconds) {}

    Any run() override {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        return begin + end;
    }

private:
    int begin;
    int end;
    int seconds;
};

int main() {
    ThreadPool threadpool;
    threadpool.setMode(PoolMode::MODE_FIXED);
    threadpool.start(1);

    Result hold = threadpool.submitTask(std::make_shared<SlowTask>(1, 1, 3));

    threadpool.submitTask(std::make_shared<SlowTask>(2, 2, 0));
    threadpool.submitTask(std::make_shared<SlowTask>(3, 3, 0));
    threadpool.submitTask(std::make_shared<SlowTask>(4, 4, 0));

    int value = hold.get().cast_<int>();

    std::cout << "hold = " << value << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}