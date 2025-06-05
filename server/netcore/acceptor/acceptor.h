#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include <cstdint>
#include <sys/socket.h>

/**
 * 仅支持IPV4
 */
class Acceptor
{
public:
    Acceptor(uint16_t port, int acceptQueLen = 8, bool forceClose = true, bool blocked = true);
    ~Acceptor();

    Acceptor(const Acceptor&) = delete;
    Acceptor(Acceptor&&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;
    Acceptor& operator=(Acceptor&&) = delete;

    int getListenFd() const;
    int getNewConnFd() const;
    int getNewConnFd(struct sockaddr* addr, socklen_t* len) const;

    static const uint16_t MAX_PORT;
    static const uint16_t MIN_PORT;
    static int WAIT_SEC;

private:
    bool setLinger();
    bool setNonblock();

    uint16_t port_;
    int acceptQueLen_;
    int listenfd_;

};

#endif
