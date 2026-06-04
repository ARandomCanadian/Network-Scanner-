#include <iostream>
#include <winsock2.h>

#include "Scanner.h"
#include "ReportManager.h"
#include "Host.h"
#include "PortInfo.h"

void start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return;
    }

    Scanner scanner;
    ReportManager reportManager;

    std::cout << "Project Started\n";
    
    scanner.discoverHostsThreaded();

    std::cout <<"starting scan ports\n";

    for (Host& host : scanner.getHost()) {
        scanner.scanPorts(host);
    }

    for (Host hosts : scanner.getHost()) {
        std::cout << hosts.ip << " : " << hosts.hostname << " : " << hosts.online << "\n";

        for (PortInfo port : hosts.openPorts) {
            std::cout << static_cast<int>(port.state) << " : " << port.port << " : " << port.banner << " : " << port.service << "\n";
        }
    }

    WSACleanup();
}

void shutdown() {

}

int main (){
    start();
}