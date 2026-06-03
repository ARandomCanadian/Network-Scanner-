#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <ws2tcpip.h>

#include "SocketClient.h"
#include "PortInfo.h"

PortInfo::PortState SocketClient::connectToPort(std::string ip, uint16_t port){
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (sock == INVALID_SOCKET) {
        return PortInfo::PortState::ERR;
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &target.sin_addr) != 1) {
        closesocket(sock);
        return PortInfo::PortState::ERR;
    }

    int result = connect(sock, (sockaddr*)&target, sizeof(target));

    if (result == 0) {
        closesocket(sock);
        return PortInfo::PortState::OPEN;
    }

    int error = WSAGetLastError();

    closesocket(sock);

    switch (error){
        case WSAECONNREFUSED:
            return PortInfo::PortState::CLOSED;
        
        case WSAETIMEDOUT:
            return PortInfo::PortState::FILTERED;

        case WSAEHOSTDOWN:
        case WSAENETUNREACH:
        case WSAEHOSTUNREACH:
            return PortInfo::PortState::UNREACHABLE;
        
        default:
            return PortInfo::PortState::ERR;
    }
}