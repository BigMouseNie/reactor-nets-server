#ifndef _EVENTDEMUX_H_
#define _EVENTDEMUX_H_

#include <cstdint>

enum EVENTTYPE {
    None        = 0x00,
    Read        = 0x01,
    Write       = 0x02,
    Error       = 0x04,
};
using EVENTS = uint32_t;
using Handle = int;

class EventDemux
{
public:
    virtual ~EventDemux() = default;
    virtual bool add(Handle fd, EVENTS events) = 0;
    virtual bool del(Handle fd) = 0;
    virtual bool modify(Handle fd, EVENTS events) = 0;

    /**
     * @brief 类似epoll_wait 或 select
     * @return 成功时返回发生事件的文件描述符数，失败时返回-1
     */
    virtual int waitEvent(int timeoutMS) = 0;

    /**
     * @brief 获取当前要处理的文件描述符
     * @param events 返回的文件描述符要处理的事件
     * @return 要处理的文件描述符
     */
    virtual Handle getNext(EVENTS* events = nullptr) = 0;
};

#endif
