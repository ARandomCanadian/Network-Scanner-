#include <iostream>
#include <winsock2.h>

#include "ServiceDetection.h"

// Constructor is currently empty, but it gives a place to load known services later.
ServiceDetection::ServiceDetection() {}

// Connects to an open port and tries to read the service banner.
// Some services send text immediately, such as SSH or FTP.
std::string ServiceDetection::grabBanner(const std::string& ip, int port){
    // Create a TCP socket for banner grabbing.
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        return "";
    }

    // Build the target address from the IP and port.
    sockaddr_in target {};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (sockaddr*)&target, sizeof(target)) == SOCKET_ERROR) {
        closesocket(sock);
        return "";
    }

    // Prevent recv from waiting forever if the service does not send a banner.
    DWORD timeout = 100;
    setsockopt(
        sock,
        SOL_SOCKET,
        SO_RCVTIMEO,
        (const char*)&timeout,
        sizeof(timeout)
    );

    char buffer[1024];

    // Try to receive banner text from the service.
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);

    closesocket(sock);

    if (bytesReceived <= 0) {
        return "";
    }

    buffer[bytesReceived] = '\0';

    return std::string(buffer);
}

// Detects the service name using either the banner text or a common port number.
std::string ServiceDetection::detectService(const std::string banner, int port) {
    // If there is no banner, the port number fallback below may still identify it.
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

    // Fallback guesses based on common default port numbers.
    switch (port) {
        case 21: return "FTP?";
        case 22: return "SSH?";
        case 80: return "HTTP?";
        case 443: return "HTTPS?";
        case 3389: return "RDP?";
    }

    return "Unknown";
}