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

    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        closesocket(sock);
        return PortInfo::PortState::ERR;
    }

    // A successful connection means the port is open.
    int result = connect(sock, (sockaddr*)&target, sizeof(target));

    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();

        if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
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
    }

    fd_set writeSet;
    fd_set exceptSet;

    FD_ZERO(&writeSet);
    FD_ZERO(&exceptSet);

    FD_SET(sock, &writeSet);
    FD_SET(sock, &exceptSet);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    result = select(0, nullptr, &writeSet, &exceptSet, &timeout);

    if (result > 0) {
        int sockerror = 0;
        int len = sizeof(sockerror);

        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&sockerror, &len);

        closesocket(sock);

        if (sockerror == 0) {
            return PortInfo::PortState::OPEN;
        }

        switch (sockerror){
            case WSAECONNREFUSED:
                return PortInfo::PortState::CLOSED;
            case WSAETIMEDOUT:
                return PortInfo::PortState::FILTERED;
        
            case WSAEHOSTDOWN:
            case WSAENETUNREACH:
            case WSAEHOSTUNREACH:
                return PortInfo::PortState::UNREACHABLE;
            default:
            return PortInfo::PortState::FILTERED;
        }
    }

    closesocket(sock);
    if (result == 0) {
        return PortInfo::PortState::FILTERED;
    }

    return PortInfo::PortState::ERR;
}