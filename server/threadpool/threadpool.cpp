#include "threadpool.h"

#include <iostream>

ThreadPool::ThreadPool(size_t threadNum)
{
    for (size_t i = 0; i < threadNum; ++i) {
        workers_.emplace_back([this](){work();});
    }

    std::cout << "ThreadPool(created successfully!)" << std::endl;
}

ThreadPool::~ThreadPool()
{
    taskQue_.cancelBlock();
    for (auto& t : workers_) {
        if (t.joinable()) {t.join();}
    }
}

void ThreadPool::addTask(Task&& task)
{
    taskQue_.push(std::forward<Task>(task));
}

void ThreadPool::work()
{
    while (true) {
        auto taskOpt = taskQue_.pop();
        if (!taskOpt) {
            break;  // cancelBlock 被调用，线程退出
        }

        // 执行任务
        (*taskOpt)();
    }
}
