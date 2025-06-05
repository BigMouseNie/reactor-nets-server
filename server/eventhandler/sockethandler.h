#ifndef _SOCKETHANDLER_H_
#define _SOCKETHANDLER_H_

#include "eventhandler.h"

struct Buffer
{
    char* data;
    char* readptr;  // 读取指针
    char* writeptr;
    int readable;   // 可读的字节
    int size;

    Buffer() : data(nullptr), readptr(nullptr), writeptr(nullptr), size(0), readable(0)
    {}

    ~Buffer()
    {
        if (data) {delete[] data;}
        data = nullptr;
        readptr = nullptr;
        writeptr = nullptr;
    }

    void init(int bufSize)
    {
        data = new char[bufSize];
        readptr = data;
        writeptr = data;
        readable = 0;
        size = bufSize;
    }

    void reset()
    {
        readptr = data;
        writeptr = data;
        readable = 0;
    }
};

class SocketHandler : public EventHandler
{
public:
    SocketHandler(int fd, bool isET, int bufSize = 1024);
    virtual ~SocketHandler();

    virtual int handleRead(int* error) override;
    virtual int handleWrite(int* error) override;
    virtual int handleError(int* error) override;

    virtual bool process() override;
    virtual int getHandler() override;
    virtual void closeEvent() override;

private:
    int eTRead(int* error);
    int noETRead(int* error);
    int eTWrite(int* error);
    int noETWrite(int* error);

    bool isClose_;
    Buffer readBuf_;
    Buffer writeBuf_;
    int fd_;
    bool isET_;
    static const int MIN_BUF_SIZE;
};

#endif
