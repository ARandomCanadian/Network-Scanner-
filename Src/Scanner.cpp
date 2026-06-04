#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#include "Scanner.h"
#include "Host.h"
#include "SocketClient.h"
#include "ServiceDetection.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

void Scanner::scanPorts(Host& host) {
    std::mutex portMutex;
    std::vector<std::thread> threads;

    const int threadcount = 64;

    int portsPerThread = 1024/threadcount;

    for (int t = 0; t< threadcount; t++) {
        int startPort = t * portsPerThread + 1;

        int endPort = (t == threadcount - 1) ? 1024 : startPort + portsPerThread - 1;

        threads.emplace_back(
            [&, startPort, endPort]() {
                ServiceDetection servDetect;

                for (int port = startPort; port <= endPort; port++) {
                    SocketClient sockClient;
                    PortInfo result = searchPort(host.ip, sockClient, port);

                    if (result.state == PortInfo::PortState::OPEN) {
                        result.banner = servDetect.grabBanner(host.ip, port);

                        result.service = servDetect.detectService(result.banner, port);

                        std::lock_guard<std::mutex> lock(portMutex);

                        host.openPorts.push_back(result);
                    }
                }
            });
    }
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

}

PortInfo Scanner::searchPort(std::string ip, SocketClient& sockClient, int port) {
    PortInfo::PortState state = sockClient.connectToPort(ip, port);

    return PortInfo(state, port);
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

void Scanner::discoverRange(std::string subnet, int startIp, int endIp) {
    for (int currentIp = startIp; currentIp <= endIp; currentIp++)
    {
        std::string ip = subnet + std::to_string(currentIp);

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

void Scanner::discoverHostsThreaded(std::string subnet) {
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
            &Scanner::discoverRange,
            this,
            subnet,
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

std::vector<Host>& Scanner::getHost() {
    return hosts;
}

const std::vector<Host>& Scanner::getHost() const {
    return hosts;
}