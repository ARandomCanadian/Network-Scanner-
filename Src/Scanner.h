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
        void discoverRange(std::string subnet, int startIP, int endIP);
        void discoverHostsThreaded(std::string subnet);
        std::vector<Host>& getHost();
        const std::vector<Host>& getHost() const;
        static void quickSort(std::vector<Host>& arr, int low, int high);
        static int partition(std::vector<Host>& arr, int low, int high);
};