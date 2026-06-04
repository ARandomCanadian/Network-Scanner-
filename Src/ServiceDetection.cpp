#include <iostream>
#include <winsock.h>

#include "ServiceDetection.h"

ServiceDetection::ServiceDetection() {

}

std::string ServiceDetection::grabBanner(const std::string& ip, int port){
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        return "";
    }

    sockaddr_in target {};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (sockaddr*)&target, sizeof(target)) == SOCKET_ERROR) {
        closesocket(sock);
        return "";
    }

    DWORD timeout = 2000;
    setsockopt(
        sock,
        SOL_SOCKET,
        SO_RCVTIMEO,
        (const char*)&timeout,
        sizeof(timeout)
    );

    char buffer[1024];

    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);

    closesocket(sock);

    if (bytesReceived <= 0) {
        return "";
    }

    buffer[bytesReceived] = '\0';

    return std::string(buffer);
}

std::string ServiceDetection::detectService(const std::string banner, int port) {
    if (banner.empty())
        return "Unknown";

    if (banner.find("SSH") != std::string::npos)
        return "SSH";
        
    if (banner.find("FTP") != std::string::npos)
        return "FTP";

    if (banner.find("SMTP") != std::string::npos)
        return "SMTP";

    if (banner.find("POP3") != std::string::npos)
        return "POP3";

    if (banner.find("IMAP") != std::string::npos)
        return "IMAP";

    switch (port) {
        case 21: return "FTP?";
        case 22: return "SSH?";
        case 80: return "HTTP?";
        case 443: return "HTTPS?";
        case 3389: return "RDP?";
    }

    return "Unknown";
}