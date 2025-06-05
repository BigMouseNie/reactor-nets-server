#ifndef _EVENTHANDLER_H_
#define _EVENTHANDLER_H_

class EventHandler
{
public:
    virtual ~EventHandler() = default;
    /**
     * @brief
     * @param error 失败时填充错误码
     * @return 成功时返回读取的字节数，失败时返回-1,error如果不为null则填充错误码
     */
    virtual int handleRead(int* error) = 0;
    virtual int handleWrite(int* error) = 0;
    virtual int handleError(int* error) = 0;

    /**
     * @brief 处理业务
     * @return 如果返回true表示业务处理完毕可以写了，返回false处理业务的数据不够需要继续读
     */
    virtual bool process() = 0;
    virtual int getHandler() = 0;
    virtual void closeEvent() = 0;
};

#endif
