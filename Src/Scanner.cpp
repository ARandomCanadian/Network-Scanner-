#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#include "Scanner.h"
#include "Host.h"
#include "PortResult.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

void Scanner::scanPorts() {

}

bool Scanner::pingHost(const char* ipAddress) {
    HANDLE hIcmp = IcmpCreateFile();

    if (hIcmp == INVALID_HANDLE_VALUE) {
        return false;
    }

    char sendData[] = "Ping Test";

    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);

    void* relayBuffer = malloc(replySize);

    DWORD result = IcmpSendEcho(
        hIcmp,
        inet_addr(ipAddress),
        sendData,
        sizeof(sendData),
        NULL,
        relayBuffer,
        replySize,
        1000
    );

    free(relayBuffer);
    IcmpCloseHandle(hIcmp);

    return (result > 0);
}

void Scanner::sortResults() {

}

PortResult Scanner::searchPort(int port) {

}

void Scanner::recursiveScan(int currentIp) {
    if (currentIp > 254)
        return;
    
    std::string ip = "192.168.56." + std::to_string(currentIp);

    std::cout << "Scanning " << ip << std::endl;

    if (Scanner::pingHost(ip.c_str())) {
        std::cout << ip << " is online \n";
    } else {
        std::cout << ip << " not online\n";
    }

    recursiveScan(currentIp + 1);
}
