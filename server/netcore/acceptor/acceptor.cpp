#include <iostream>
#include <stdexcept>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "acceptor.h"

const uint16_t Acceptor::MAX_PORT = 65535;
const uint16_t Acceptor::MIN_PORT = 1024;
int Acceptor::WAIT_SEC = 3;

Acceptor::Acceptor(uint16_t port, int acceptQueLen, bool forceClose, bool blocked)
    : port_(port), acceptQueLen_(acceptQueLen)
{
    if (port_ < MIN_PORT || port_ > MAX_PORT) {
        throw std::invalid_argument("Acceptor(Invalid port!)");
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listenfd_ = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd_ < 0) {
        perror("socket failed");
        throw std::runtime_error("Acceptor(socket creation failed!)");
    }

    socklen_t len = sizeof(addr);
    if (bind(listenfd_, (sockaddr*)&addr, len) == -1) {
        close(listenfd_);
        perror("bind failed");
        throw std::runtime_error("Acceptor(listen file descriptor binding failed!)");
    }

    if (!forceClose && !setLinger()) {
        close(listenfd_);
        throw std::runtime_error("Acceptor(setting linger failed!)");
    }

    if (!blocked && !setNonblock()) {
        close(listenfd_);
        throw std::runtime_error("Acceptor(setting non-blocking failed!)");
    }

    if (listen(listenfd_, acceptQueLen_) == -1) {
        close(listenfd_);
        perror("listen failed");
        throw std::runtime_error("Acceptor(listen failed!)");
    }
    
    std::cout << "Acceptor(created successfully!)" << std::endl;
}

Acceptor::~Acceptor()
{
    close(listenfd_);
}

int Acceptor::getListenFd() const
{
    return listenfd_;
}

int Acceptor::getNewConnFd() const
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    return getNewConnFd((struct sockaddr*)&addr, &len);
}

int Acceptor::getNewConnFd(struct sockaddr* addr, socklen_t* len) const
{
    return accept(listenfd_, (struct sockaddr *)&addr, len);
}

bool Acceptor::setLinger()
{
    struct linger optLinger;
    optLinger.l_onoff = 1;  // 需要延时
    optLinger.l_linger = WAIT_SEC;  // 最多等待秒数
    int ret = setsockopt(listenfd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret == -1){
        perror("setsockopt failed");
        return false;
    }
    return true;
}

bool Acceptor::setNonblock()
{
    int flags = fcntl(listenfd_, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        return false;
    }

    flags |= O_NONBLOCK;  // 设置非阻塞标志
    if (fcntl(listenfd_, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL failed");
        return false;
    }

    return true;
}