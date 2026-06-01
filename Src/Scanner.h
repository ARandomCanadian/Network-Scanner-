#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#include "Host.h"
#include "PortResult.h"

class Scanner {
    private:
        std::string targetIP;
        int startPort;
        int endPort;
        std::vector<Host> hosts;
        std::mutex hostMutex;
    
    public:
        void scanPorts();
        bool pingHost(const char* host);
        void sortResults();
        PortResult searchPort(int port);
        void scanRange(int startIP, int endIP);
        void thrededScan();
};