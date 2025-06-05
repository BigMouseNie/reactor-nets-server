#include "subreactor.h"
#include "epolldemux.h"
#include "sockethandler.h"
#include "threadpool.h"
#include "sockethandler.h"

#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

bool SubReactor::init(std::shared_ptr<ThreadPool> threadpool, int eventLen, bool nonBlocked)
{
    if (initialized_) {
        return true;
    }

    threadpool_ = threadpool;
    nonBlocked_ = nonBlocked;
    
    try {
        demux_.reset(new EpollDemux(eventLen, nonBlocked));
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    initialized_ = true;
    return true;
}

void SubReactor::addClient(int fd)
{
    if (nonBlocked_ && !setFdNonblock(fd)) {
        std::cerr << "SubReactor (Setting to non-blocking fd(" << fd << ") failed)" << std::endl;
        close(fd);
        return;
    }
    if (!demux_->add(fd, EVENTTYPE::Read | EVENTTYPE::Error)) {
    // if (!demux_->add(fd, EVENTTYPE::Read)) {
        std::cerr << "SubReactor(demux_ -> add fd(" << fd << ") failed)" << std::endl;
        close(fd);
        return;
    }

    auto sptr = std::make_shared<SocketHandler>(fd, nonBlocked_, 128);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        eventHandlerMap_[fd] = sptr;
    }
    ehCnt_.fetch_add(1);
    std::cout << "SubReactor(addClient fd(" << fd << ") successful!)" << std::endl;
}

void SubReactor::removeClient(int fd)
{
    demux_->del(fd);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = eventHandlerMap_.find(fd);
    if (it != eventHandlerMap_.end()) {
        it->second->closeEvent();
        eventHandlerMap_.erase(it);
        ehCnt_.fetch_sub(1);
    }
}

void SubReactor::removeClient(std::shared_ptr<EventHandler> speh)
{
    int fd = speh->getHandler();
    demux_->del(fd);
    speh->closeEvent();
    std::lock_guard<std::mutex> lock(mutex_);
    eventHandlerMap_.erase(fd);
    ehCnt_.fetch_sub(1);
}

int SubReactor::setFdNonblock(int fd)
{
    std::cout << "SubReactor(setFdNonblock fd("<< fd <<"))" << std::endl;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        return false;
    }

    flags |= O_NONBLOCK;  // 设置非阻塞标志
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL failed");
        return false;
    }

    return true;
}

int SubReactor::getCount()
{
    return ehCnt_.load();
}

void SubReactor::eventLoop()
{
    if (!initialized_) {
        std::cerr << "SubReactor(not initialized, please call init()!)" << std::endl;
        return;
    }

    int timeoutMS = 5000;  // 5 sec
    while (true) {
        int events = demux_->waitEvent(timeoutMS);
        EVENTS eventFlag = EVENTTYPE::None;
        int fd;
        std::cout << "SubReactor(eventLoop events(" << events << "))" << std::endl;
        while ((fd = demux_->getNext(&eventFlag)) != -1) {
            if (eventFlag & EVENTTYPE::Error) {
                std::cerr << "SubReactor(eventFlag & EVENTTYPE::Error)" << std::endl;
                removeClient(fd);
            } else if (eventFlag & EVENTTYPE::Read) {
                std::shared_ptr<EventHandler> speh;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    auto it = eventHandlerMap_.find(fd);
                    if (it != eventHandlerMap_.end()) {
                        speh = it->second;
                    } else {
                        std::cerr << "SubReactor(eventHandlerMap_ find error!)" << std::endl;
                        continue;
                    }
                }
                dealRead(speh);
            } else if (eventFlag & EVENTTYPE::Write){
                std::shared_ptr<EventHandler> speh;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    auto it = eventHandlerMap_.find(fd);
                    if (it != eventHandlerMap_.end()) {
                        speh = it->second;
                    }  else {
                        std::cerr << "SubReactor(eventHandlerMap_ find error!)" << std::endl;
                        continue;
                    }
                }
                dealWrite(speh);
            }
        }
    }
}

void SubReactor::dealRead(std::shared_ptr<EventHandler> speh)
{
    std::cout << "SubReactor(dealRead, Add fd("<< speh->getHandler() <<") read events to the thread pool)" << std::endl;
    threadpool_->addTask(std::bind(&SubReactor::onRead, this, speh));
}

void SubReactor::dealWrite(std::shared_ptr<EventHandler> speh)
{
    threadpool_->addTask(std::bind(&SubReactor::onWrite, this, speh));
}

void SubReactor::onRead(std::shared_ptr<EventHandler> speh)
{
    int ret = -1;
    int readErrno = 0;
    ret = speh->handleRead(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        removeClient(speh);
        return;
    }
    std::cout << "SubReactor(onRead readErrno(" << readErrno << "))" << std::endl;
    onProcess(speh);
}

void SubReactor::onWrite(std::shared_ptr<EventHandler> speh)
{
    int ret = -1;
    int writeErrno = 0;
    ret = speh->handleWrite(&writeErrno);
    if (ret <= 0) {
        if (writeErrno == EAGAIN) {
            demux_->modify(speh->getHandler(), EVENTTYPE::Write | EVENTTYPE::Error);
            // demux_->modify(speh->getHandler(), EVENTTYPE::Write);
            return;
        } else {
            removeClient(speh);
            return;
        }
    }
    onProcess(speh);
}

void SubReactor::onProcess(std::shared_ptr<EventHandler> speh) {
    if(speh->process()) {
        // EventHandler 处理完毕可以监听写了
        std::cout << "SubReactor(EventHandler process is true, modify EVENTTYPE::Write | EVENTTYPE::Error)" << std::endl;
        demux_->modify(speh->getHandler(), EVENTTYPE::Write | EVENTTYPE::Error);
        // demux_->modify(speh->getHandler(), EVENTTYPE::Write);
    } else {
        // EventHandler 处理未完毕继续读
        std::cout << "SubReactor(EventHandler process is false, modify EVENTTYPE::Read | EVENTTYPE::Error)" << std::endl;
        demux_->modify(speh->getHandler(), EVENTTYPE::Read | EVENTTYPE::Error);
        // demux_->modify(speh->getHandler(), EVENTTYPE::Read);
    }
}
