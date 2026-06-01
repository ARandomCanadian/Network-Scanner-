#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <ws2tcpip.h>

#include "SocketClient.h"

bool SocketClient::connectToPort(std::string ip, uint16_t port){
    std::cout << "connecting\n";
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (sock == INVALID_SOCKET) {
        std::cout << "failed\n";
        return false;
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);

    inet_pton(AF_INET, ip.c_str(), &target.sin_addr);

    int result = connect(sock, (sockaddr*)&target, sizeof(target));

    std::cout << "connected\n";

    closesocket(sock);

    return result == 0;
}