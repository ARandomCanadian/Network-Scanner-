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

void scanPorts() {

}

bool pingHosts(const char* host) {

}

void sortResults() {

}

PortResult searchPorts() {

}

void recursiveScan(int currentIp) {
    if (currentIp > 254)
        return;
    
    std::string ip = "192.168.56." + std::to_string(currentIp);

    std::cout << "Scanning " << ip << std::endl;

    if (pingHosts(ip.c_str())) {
        std::cout << ip << "is online \n";
    }

    recursiveScan(currentIp ++);
}
