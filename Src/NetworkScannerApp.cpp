#include <iostream>
#include <winsock2.h>

#include "Scanner.h"
#include "ReportManager.h"

void start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return;
    }

    Scanner scanner;
    ReportManager reportManager;

    std::cout << "Project Started\n";
    
    scanner.thrededScan();

    std::cout <<"starting scan ports";

    scanner.scanPorts();

    WSACleanup();
}

void shutdown() {

}

int main (){
    start();
}