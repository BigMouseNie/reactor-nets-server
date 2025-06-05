#ifndef _SUBREACTOR_H_
#define _SUBREACTOR_H_

#include <memory>
#include <atomic>
#include <unordered_map>
#include <mutex>

class ThreadPool;
class EventHandler;
class EventDemux;

class SubReactor
{
public:
    SubReactor() = default;  
    ~SubReactor() = default;
    bool init(std::shared_ptr<ThreadPool> threadpool, int eventLen, bool nonBlocked);
    int getCount();
    void addClient(int fd);
    void removeClient(int fd);
    void removeClient(std::shared_ptr<EventHandler> speh);
    void eventLoop();

private:
    void dealRead(std::shared_ptr<EventHandler> speh);
    void dealWrite(std::shared_ptr<EventHandler> speh);
    void onRead(std::shared_ptr<EventHandler> speh);
    void onWrite(std::shared_ptr<EventHandler> speh);
    void onProcess(std::shared_ptr<EventHandler> speh);
    int setFdNonblock(int fd);

    bool nonBlocked_;
    bool initialized_ = false;
    std::unique_ptr<EventDemux> demux_;
    std::shared_ptr<ThreadPool> threadpool_;
    std::atomic<int> ehCnt_;
    std::mutex mutex_;
    std::unordered_map<int, std::shared_ptr<EventHandler>> eventHandlerMap_;
};

#endif
