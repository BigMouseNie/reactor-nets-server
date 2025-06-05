#ifndef _EPOLLDEMUX_H_
#define _EPOLLDEMUX_H_

#include <vector>
#include <cstdint>

#include "eventdemux.h"

class EpollDemux : public EventDemux
{
public:
    explicit EpollDemux(int maxEvent, bool isET);
    virtual ~EpollDemux();

    // 线程安全
    virtual bool add(Handle fd, EVENTS events) override;
    virtual bool del(Handle fd) override;
    virtual bool modify(Handle fd, EVENTS events) override;
    
    virtual int waitEvent(int timeoutMS) override;
    virtual Handle getNext(EVENTS* events = nullptr) override;

private:
    uint32_t parseEVENTS(EVENTS events) const;
    EVENTS getEVENTS(uint32_t flag) const;

    bool isET_;
    int epollfd_;
    int maxEvent_;
    int eventQueLen_ = 0;
    int curIdx_ = 0;
    std::vector<struct epoll_event> eventQue_;
    static const uint32_t ERROR_FLAG;
};

#endif
