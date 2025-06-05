#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "blockque.h"

#include <functional>
#include <thread>
#include <vector>

using Task = std::function<void()>;

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadNum);
    ~ThreadPool();

    void addTask(Task&& task);

private:
    void work();

    BlockQue<Task> taskQue_;    // 线程安全
    std::vector<std::thread> workers_;
};

#endif
