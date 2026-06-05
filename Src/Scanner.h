#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#include "Host.h"
#include "SocketClient.h"

// Main class that controls network discovery, port scanning, sorting, and searching.
class Scanner {
    private:
        // Optional scan settings kept as class fields for future expansion.
        std::string targetIP;
        int startPort;
        int endPort;

        // All hosts discovered during the network scan.
        std::vector<Host> hosts;

        // Protects the hosts vector when multiple discovery threads add hosts at once.
        std::mutex hostMutex;
    
    public:
        // Scans ports 1-1024 on one host using multiple threads.
        void scanPorts(Host& host);

        // Sends an ICMP ping to check whether a host is online.
        bool pingHost(const char* host);

        // Sorts hosts by the number of open ports using quick sort.
        void sortResults();

        // Scans one specific port and returns the result.
        PortInfo searchPort(std::string ip, SocketClient& sockClient, int port);

        // Scans one section of a subnet. Used by the threaded discovery method.
        void discoverRange(std::string subnet, int startIP, int endIP);

        // Splits the subnet scan across multiple threads for faster discovery.
        void discoverHostsThreaded(std::string subnet);

        // Returns editable access to the host list.
        std::vector<Host>& getHost();

        // Returns read-only access to the host list.
        const std::vector<Host>& getHost() const;

        // Recursive quick sort used to meet the sorting and recursion requirements.
        static void quickSort(std::vector<Host>& arr, int low, int high);

        // Places the pivot in the correct position for quick sort.
        static int partition(std::vector<Host>& arr, int low, int high);

        // Sorts a single host's open ports by port number before binary search.
        void sortPortsByNumber(Host& host);

        // Binary search for an open port after the port list has been sorted.
        int binarySearchPort(const Host& host, int targetPort); 
};
