#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#include "Host.h"
#include "SocketClient.h"

class Scanner {
    private:
        std::string targetIP;
        int startPort;
        int endPort;
        std::vector<Host> hosts;
        std::mutex hostMutex;
    
    public:
        void scanPorts(Host& host);
        bool pingHost(const char* host);
        void sortResults();
        PortInfo searchPort(std::string ip, SocketClient& sockClient, int port);
        void discoverRange(int startIP, int endIP);
        void discoverHostsThreaded();
        std::vector<Host>& getHost();
        const std::vector<Host>& getHost() const;
};