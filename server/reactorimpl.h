#ifndef _REACTORIMPL_H_
#define _REACTORIMPL_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <atomic>

class Acceptor;
class EventDemux;
class ThreadPool;
class SubReactor;

class ReactorImpl
{
public:
    static ReactorImpl* getInstance();
    static void destroyInstance(); // 添加销毁函数
    bool init(uint16_t port, int listenLen, bool listenCloseDelay,
              bool nonBlocked, int eventLen, int threadNum, int subRectorNum);
    void eventLoop();
    void stopEventLoop();
    ~ReactorImpl();

private:
    explicit ReactorImpl() = default;
    void dealListen();
    void addClient(int fd);
    int setFdNonblock(int fd);
    
    static ReactorImpl* instance_;
    bool nonBlocked_;
    bool initialized_ = false;
    std::atomic<bool> runing_ = false;
    size_t nextReactor_ = 0;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventDemux> mainDemux_; // 仅监听acceptor_
    std::shared_ptr<ThreadPool> threadpool_;
    std::vector<std::shared_ptr<SubReactor>> subReactors_;
};

#endif
