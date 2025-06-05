#include <cstring>
#include <iostream>

#include <unistd.h>
#include <errno.h>

#include "sockethandler.h"

const int SocketHandler::MIN_BUF_SIZE = 64;

SocketHandler::SocketHandler(int fd, bool isET, int bufSize)
    :EventHandler(), fd_(fd), isET_(isET), isClose_(false)
{
    if (bufSize < MIN_BUF_SIZE) {
        bufSize = MIN_BUF_SIZE;
    }

    readBuf_.init(bufSize);
    writeBuf_.init(bufSize);
}

SocketHandler::~SocketHandler()
{
    closeEvent();
}

int SocketHandler::handleRead(int* error)
{
    if (isET_) {
        return eTRead(error);
    }
    return noETRead(error);
}

int SocketHandler::handleWrite(int* error)
{
    if (isET_) {
        return eTWrite(error);
    }
    return noETWrite(error);
}

int SocketHandler::handleError(int* error)
{
    // pass
    return 0;
}

bool SocketHandler::process()   // 做回声业务
{
    if (readBuf_.readable <= 0) {
        return false;
    }

    std::memcpy(writeBuf_.writeptr, readBuf_.readptr, readBuf_.readable);
    writeBuf_.readable += readBuf_.readable;
    writeBuf_.writeptr += readBuf_.readable;
    readBuf_.reset();
    return true;
}

int SocketHandler::getHandler()
{
    return fd_;
}

void SocketHandler::closeEvent()
{
    if (!isClose_) {
        close(fd_);
    }
    isClose_ = true;
}

int SocketHandler::eTRead(int* error)
{
    std::cout << "SocketHandler(eTRead fd("<< fd_ <<"))" << std::endl;;
    *error = 0;
    ssize_t total = 0;
    while (readBuf_.size - readBuf_.readable > 0) {
        ssize_t n = read(fd_, readBuf_.writeptr, readBuf_.size - readBuf_.readable);
        if (n > 0) {
            total += n;
            readBuf_.readable += n;
            readBuf_.writeptr += n;
        } else if (n == 0) {
            // 对端关闭连接
            return 0;
        } else {
            if (errno == EINTR) {
                // 被信号中断，继续读
                continue;
            } else {
                // 其他错误
                *error = errno; 
                std::cout << "SocketHandler(eTRead tatal("<< total <<"),data("<< readBuf_.data << "))" << std::endl;;
                return -1;
            }
        }
    }

    std::cout << "SocketHandler(eTRead tatal("<< total <<"),data("<< readBuf_.data <<"))" << std::endl;;
    return total;
}

int SocketHandler::noETRead(int* error)
{
    *error = 0;
    ssize_t n = read(fd_, readBuf_.writeptr, readBuf_.size - readBuf_.readable);

    if (n == 0) {
        // 对端关闭连接
        return 0;
    } else if (n < 0){
        // 发生错误
        *error = errno;
        return -1;
    }
    readBuf_.writeptr += n;
    readBuf_.readable += n;
    return n;
}

int SocketHandler::eTWrite(int* error)
{
    *error = 0;
    ssize_t total = 0;
    while (writeBuf_.readable > 0) {
        ssize_t n = write(fd_, writeBuf_.readptr, writeBuf_.readable);
        if (n > 0) {
            total += n;
            writeBuf_.readptr += n;
            writeBuf_.readable -= n;
        } else {
            if (errno == EINTR) {
                continue;
            } else {
                *error = errno; // 如果errno为EAGIN,上层会继续调用写并且当前writeBuf_状态已保存
                return -1;
            }
        }
    }

    writeBuf_.reset();  // 写完毕
    return total;
}

int SocketHandler::noETWrite(int* error)
{
    *error = 0;
    ssize_t total = 0;
    while (writeBuf_.readable > 0) {
        ssize_t n = write(fd_, writeBuf_.readptr, writeBuf_.readable);
        if (n <= 0) {
            if (errno == EINTR) {
                continue;  // 被信号中断，继续写
            }
            *error = errno;
            return -1;  // 写错误
        }
        total += n;
        writeBuf_.readptr += n;
        writeBuf_.readable -= n;
    }

    writeBuf_.reset();  // 写完毕
    return total;
}
