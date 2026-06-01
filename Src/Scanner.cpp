#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#include "Scanner.h"
#include "Host.h"
#include "PortResult.h"
#include "SocketClient.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

void Scanner::scanPorts() {
    std::cout << "Starting SockClient\n";

    SocketClient sockClient;

    std::cout << "Starting Loop\n";

    for (Host h : hosts) {
        std::cout << "connecting to port 24 on " << h.ip << "\n";
        sockClient.connectToPort(h.ip, 24);
    }
}

PortResult Scanner::searchPort(int port) {

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
        50
    );

    free(relayBuffer);
    IcmpCloseHandle(hIcmp);

    return (result > 0);
}

void Scanner::sortResults() {

}

void Scanner::scanRange(int startIp, int endIp) {
    for (int currentIp = startIp; currentIp <= endIp; currentIp++)
    {
        std::string ip = "192.168.56." + std::to_string(currentIp);

        if (pingHost(ip.c_str())){
            Host host(ip, true);

            {
                std::lock_guard<std::mutex> lock(hostMutex);
                hosts.push_back(host);
            }

            std::cout << ip << " is online\n";

        }
    }
}

void Scanner::thrededScan() {
    const int firstIp = 1;
    const int lastIp = 255;

    unsigned int threadCount = std::thread::hardware_concurrency();

    if (threadCount == 0) {
        threadCount = 8;
    }

    std::vector<std::thread> threads;

    int totalIps = lastIp - firstIp + 1;
    int chunkSize = totalIps / threadCount;

    int start = firstIp;

    for (unsigned int i = 0; i < threadCount; i++) {
        int end;

        if (i == threadCount - 1) {
            end = lastIp;
        } else {
            end = start + chunkSize - 1;
        }

        threads.emplace_back(
            &Scanner::scanRange,
            this,
            start,
            end
        );

        start = end + 1;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "\nScan Complete.\n";
}