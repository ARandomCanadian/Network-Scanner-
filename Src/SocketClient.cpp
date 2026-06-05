#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <ws2tcpip.h>

#include "SocketClient.h"
#include "PortInfo.h"

// Attempts to connect to a TCP port and returns the scan state.
// This is the low-level socket code used by Scanner::searchPort.
PortInfo::PortState SocketClient::connectToPort(std::string ip, uint16_t port){
    // Create a TCP socket for the connection attempt.
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (sock == INVALID_SOCKET) {
        return PortInfo::PortState::ERR;
    }

    // Fill in the target IP address and port in the format Winsock expects.
    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &target.sin_addr) != 1) {
        closesocket(sock);
        return PortInfo::PortState::ERR;
    }

    // A successful connection means the port is open.
    int result = connect(sock, (sockaddr*)&target, sizeof(target));

    if (result == 0) {
        closesocket(sock);
        return PortInfo::PortState::OPEN;
    }

    // Save the socket error before closing the socket.
    int error = WSAGetLastError();

    closesocket(sock);

    // Convert Winsock error codes into project-friendly port states.
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