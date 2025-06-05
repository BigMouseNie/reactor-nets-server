#ifndef _BLOCKQUE_H_
#define _BLOCKQUE_H_

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * push         : pushMux_;
 * pop          : popMux_ || popMux_ -> pushMux;
 * cancelBlock  : popMux_ -> pushMux;
 */
template <typename T>
class BlockQue
{
public:
    explicit BlockQue();
    ~BlockQue();

    void push(T&& elem);
    std::optional<T> pop();
    void cancelBlock();

private:
    size_t swapQue();

    std::queue<T> pushQue_;
    std::queue<T> popQue_;
    std::mutex pushMux_;
    std::mutex popMux_;
    std::condition_variable noEmpty_;
    bool cancelBlock_;
};

template <typename T>
BlockQue<T>::BlockQue()
    : cancelBlock_(false)
{}

template <typename T>
BlockQue<T>::~BlockQue()
{
    cancelBlock();
}

template <typename T>
void BlockQue<T>::push(T&& elem)
{
    std::lock_guard<std::mutex> pushLock(pushMux_);
    pushQue_.emplace(std::forward<T>(elem));
    noEmpty_.notify_one();
}

template <typename T>
std::optional<T> BlockQue<T>::pop()
{
    std::lock_guard<std::mutex> popLock(popMux_);
    if (popQue_.empty() && swapQue() == 0) {  // cancel block 情形
        return std::nullopt;
    }   

    std::optional<T> ret(std::move(popQue_.front()));
    popQue_.pop();
    return ret;
}

template <typename T>
size_t BlockQue<T>::swapQue()  // 必须在持有popLock的情况下调用
{
    std::unique_lock<std::mutex> uiPushLock(pushMux_);
    noEmpty_.wait(uiPushLock, [this](){return (!pushQue_.empty() || cancelBlock_);});
    if (cancelBlock_) {
        return 0;
    }

    std::swap(pushQue_, popQue_);
    return popQue_.size();
}

template <typename T>
void BlockQue<T>::cancelBlock()
{
    std::lock_guard<std::mutex> popLock(popMux_);
    std::lock_guard<std::mutex> pushLock(pushMux_);
    cancelBlock_ = true;
    noEmpty_.notify_all();
}

#endif
