#include "reactorimpl.h"

#include <csignal>   // for signal
#include <iostream>

void signalHandler(int signum) {
    std::cout << "\nSignal " << signum << " received. Exiting..." << std::endl;
    ReactorImpl::getInstance()->stopEventLoop();
}

int main () {
    // 注册信号处理函数
    std::signal(SIGINT, signalHandler);   // Ctrl+C

    if (!ReactorImpl::getInstance()->init(8081, 4, false, true, 128, 4, 2)) {
        std::cout << "ReactorImpl::init() failed!" << std::endl; 
        return -1;
    }
    std::cout << "ReactorImpl::init() successful!" << std::endl; 
    std::cout << "Server running. Press Ctrl+C to stop." << std::endl;
    ReactorImpl::getInstance()->eventLoop();
    ReactorImpl::destroyInstance();  // 清理资源
}
