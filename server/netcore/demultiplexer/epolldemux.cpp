#include <stdexcept>
#include <iostream>

#include <sys/epoll.h>
#include <unistd.h>

#include "epolldemux.h"

const uint32_t EpollDemux::ERROR_FLAG = EPOLLERR | EPOLLHUP | EPOLLRDHUP;

EpollDemux::EpollDemux(int maxEvent, bool isET)
    : EventDemux(), maxEvent_(maxEvent), isET_(isET)
{
    if (maxEvent_ <= 0) {
        throw std::invalid_argument("EpollDemux(maxEvent error!)");
    }
    eventQue_.resize(maxEvent_);
    epollfd_ = epoll_create1(0);
    if (epollfd_ < 0) {
        perror("epoll_create1 error");
        throw std::runtime_error("EpollDemux(epoll_create1 error!)");
    }

    std::cout << "EpollDemux(created successfully!)" << std::endl;
}

EpollDemux::~EpollDemux()
{
    close(epollfd_);
}

bool EpollDemux::add(Handle fd, EVENTS events)
{
    if(fd < 0) return false;
    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = parseEVENTS(events);
    return 0 == epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
}

bool EpollDemux::del(Handle fd)
{
    if(fd < 0) return false;
    struct epoll_event ev = {0};
    return 0 == epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
}

bool EpollDemux::modify(Handle fd, EVENTS events)
{
    if(fd < 0) return false;
    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = parseEVENTS(events);
    return 0 == epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
}

int EpollDemux::waitEvent(int timeoutMS)
{
    if (eventQueLen_ > 0) {
        return eventQueLen_;
    }
    eventQueLen_ = epoll_wait(epollfd_, &eventQue_[0], maxEvent_, timeoutMS);
    // std::cout << "EpollDemux(waitEvent->epoll_wait-> eventQueLen_ :" << eventQueLen_ << ")" << std::endl;
    curIdx_ = 0;
    return eventQueLen_;
}

Handle EpollDemux::getNext(EVENTS* events)
{
    if (eventQueLen_ <= 0) {return -1;}
    Handle fd = eventQue_[curIdx_].data.fd;
    if (events) {
        std::cout << "EpollDemux(getNext fd("<< fd << "), original events(" << eventQue_[curIdx_].events << "))" << std::endl;
        *events = getEVENTS(eventQue_[curIdx_].events);
        std::cout << "EpollDemux(getNext fd("<< fd << "),  EVENTS(" << *events << "))" << std::endl;
    }
    --eventQueLen_;
    ++curIdx_;
    return fd;
}

uint32_t EpollDemux::parseEVENTS(EVENTS events) const
{
    uint32_t flag = 0;
    if (events & EVENTTYPE::Read) {
        flag |= EPOLLIN;
    }

    if (events & EVENTTYPE::Write) {
        flag |= EPOLLOUT;
    }

    if (events & EVENTTYPE::Error) {
        flag |= (ERROR_FLAG);
    }

    if (isET_) {
        flag |= EPOLLET;
    }

    return flag;
}

EVENTS EpollDemux::getEVENTS(uint32_t flag) const
{
    EVENTS events = EVENTTYPE::None;
    if (flag & EPOLLIN) {
        events |= EVENTTYPE::Read;
    }

    if (flag & EPOLLOUT) {
        events |= EVENTTYPE::Write;
    }

    if (flag & ERROR_FLAG) {
        events |= EVENTTYPE::Error;
    }

    return events;
}
