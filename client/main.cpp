#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);  // 服务器端口
    inet_pton(AF_INET, "192.168.195.131", &serverAddr.sin_addr);  // 服务器IP

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        close(sock);
        return 1;
    }

    std::cout << "Connected to echo server.\n";

    char buffer[1024];
    while (true) {
        std::cout << "Enter message: ";
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) break;

        // 用 write 替代 send
        ssize_t sent = write(sock, input.c_str(), input.size());
        if (sent == -1) {
            perror("write");
            break;
        }

        // 用 read 替代 recv
        ssize_t len = read(sock, buffer, sizeof(buffer) - 1);
        if (len <= 0) {
            std::cout << "Server closed connection or error occurred.\n";
            break;
        }

        buffer[len] = '\0';
        std::cout << "Echo from server: " << buffer << "\n";
    }

    close(sock);
    return 0;
}
