#include <cstdint>
#include <iostream>

#include "reactorimpl.h"
#include "acceptor.h"
#include "epolldemux.h"
#include "threadpool.h"
#include "sockethandler.h"
#include "subreactor.h"

ReactorImpl* ReactorImpl::instance_ = nullptr;

ReactorImpl::~ReactorImpl()
{}

ReactorImpl* ReactorImpl::getInstance()
{
    if (!instance_) {
        instance_ = new ReactorImpl();
    }
    return instance_;
}

void ReactorImpl::destroyInstance()
{
    delete instance_;
    instance_ = nullptr;
}

bool ReactorImpl::init(uint16_t port, int listenLen, bool listenCloseDelay,
                       bool nonBlocked, int eventLen, int threadNum, int subRectorNum)
{
    if (initialized_) {
        return true;
    }

    nonBlocked_ = nonBlocked;

    try {
        acceptor_.reset(new Acceptor(port, listenLen, !listenCloseDelay, !nonBlocked_));
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    try {
        mainDemux_.reset(new EpollDemux(2, nonBlocked));
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    
    threadpool_.reset(new ThreadPool(threadNum));

    for (int i=0; i<subRectorNum; ++i) {
        subReactors_.emplace_back(std::make_shared<SubReactor>());
    }

    for (auto subReactor : subReactors_) {
        if (!subReactor->init(threadpool_, eventLen, nonBlocked_)) {
            return false;
        }
    }

    std::cout << "ReactorImpl(init successfully!)" << std::endl;
    initialized_ = true;
    return true;
}

void ReactorImpl::stopEventLoop() {
    runing_ = false;
}

void ReactorImpl::eventLoop()
{
    if (!initialized_) {
        std::cerr << "ReactorImpl(not initialized, please call init()!)" << std::endl;
        return;
    }

    for (auto subReactor : subReactors_) {
        std::thread([subReactor] { subReactor->eventLoop(); }).detach();
    }

    mainDemux_->add(acceptor_->getListenFd(), EVENTTYPE::Read | EVENTTYPE::Error);
    int timeoutMS = 10000;  // 10 sec
    runing_ = true;
    while (runing_) {
        int events = mainDemux_->waitEvent(timeoutMS);
        EVENTS eventFlag = EVENTTYPE::None;
        while (mainDemux_->getNext(&eventFlag) != -1) {
            if (eventFlag & EVENTTYPE::Read) {
                std::cout << "ReactorImpl(eventLoop->mainDemux_->getNext->EVENTTYPE::Read)" << std::endl;
                dealListen();
            } else {
                std::cerr << "ReactorImpl(Acceptor(EVENTTYPE::Error))" << std::endl;
                return;
            }
        }
    }

}

void ReactorImpl::dealListen()
{
    do {
        int fd = acceptor_->getNewConnFd();
        if(fd <= 0) { return;}
        addClient(fd);
    } while(nonBlocked_);
}

void ReactorImpl::addClient(int fd)
{
    std::cout << "ReactorImpl(Prepare to add fd(" << fd << ") to subrector)" << std::endl;
    subReactors_[nextReactor_]->addClient(fd);
    nextReactor_ = (nextReactor_ + 1) % subReactors_.size();
}
