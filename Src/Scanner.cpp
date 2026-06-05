#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <algorithm>
#include <string>

#include "Scanner.h"
#include "Host.h"
#include "SocketClient.h"
#include "ServiceDetection.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

// Scans ports 1-1024 on a single host.
// The port range is split across threads to make scanning much faster.
void Scanner::scanPorts(Host& host) {
    std::mutex portMutex;
    std::vector<std::thread> threads;

    // Number of worker threads used for the port scan.
    const int threadcount = 64;

    int portsPerThread = 1024/threadcount;

    for (int t = 0; t< threadcount; t++) {
        int startPort = t * portsPerThread + 1;

        int endPort = (t == threadcount - 1) ? 1024 : startPort + portsPerThread - 1;

        // Each thread scans one section of the port range.
        threads.emplace_back(
            [&, startPort, endPort]() {
                ServiceDetection servDetect;

                for (int port = startPort; port <= endPort; port++) {
                    SocketClient sockClient;
                    PortInfo result = searchPort(host.ip, sockClient, port);

                    // Only store open ports because closed ports are not useful in the final report.
                    if (result.state == PortInfo::PortState::OPEN) {
                        result.banner = servDetect.grabBanner(host.ip, port);

                        result.service = servDetect.detectService(result.banner, port);

                        // Protect openPorts because multiple threads may add results at the same time.
                        std::lock_guard<std::mutex> lock(portMutex);

                        host.openPorts.push_back(result);
                    }
                }
            });
    }
    // Wait for every worker thread to finish before returning.
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

}

// Uses SocketClient to scan one port and wraps the result in a PortInfo object.
PortInfo Scanner::searchPort(std::string ip, SocketClient& sockClient, int port) {
    PortInfo::PortState state = sockClient.connectToPort(ip, port);

    return PortInfo(state, port);
}

// Pings one IP address to decide whether it should be added as an online host.
bool Scanner::pingHost(const char* ipAddress) {
    // Creates a Windows ICMP handle used to send the ping request.
    HANDLE hIcmp = IcmpCreateFile();

    if (hIcmp == INVALID_HANDLE_VALUE) {
        return false;
    }

    char sendData[] = "Ping Test";

    DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);

    void* relayBuffer = malloc(replySize);

    // Sends a short ping with a low timeout so discovery does not take too long.
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

// Scans a subsection of the subnet, such as 192.168.56.1 to 192.168.56.50.
// This method is run by multiple threads from discoverHostsThreaded.
void Scanner::discoverRange(std::string subnet, int startIp, int endIp) {
    for (int currentIp = startIp; currentIp <= endIp; currentIp++)
    {
        std::string ip = subnet + std::to_string(currentIp);

        // Only online hosts are stored in the results list.
        if (pingHost(ip.c_str())){
            Host host(ip, true);

            {
                // Protect hosts because multiple discovery threads may add hosts at once.
                std::lock_guard<std::mutex> lock(hostMutex);
                hosts.push_back(host);
            }

            std::cout << ip << " is online\n";

        }
    }
}

// Splits the full subnet into chunks and scans those chunks at the same time.
void Scanner::discoverHostsThreaded(std::string subnet) {
    const int firstIp = 1;
    const int lastIp = 255;

    // Use the CPU thread count when possible for a reasonable default.
    unsigned int threadCount = std::thread::hardware_concurrency();

    if (threadCount == 0) {
        threadCount = 8;
    }

    std::vector<std::thread> threads;

    int totalIps = lastIp - firstIp + 1;
    int chunkSize = totalIps / threadCount;

    int start = firstIp;

    // Create one worker thread per chunk of IP addresses.
    for (unsigned int i = 0; i < threadCount; i++) {
        int end;

        if (i == threadCount - 1) {
            end = lastIp;
        } else {
            end = start + chunkSize - 1;
        }

        // Each thread scans one section of the port range.
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

// Gives the main program editable access to the stored host list.
std::vector<Host>& Scanner::getHost() {
    return hosts;
}

const // Gives the main program editable access to the stored host list.
std::vector<Host>& Scanner::getHost() const {
    return hosts;
}

// Starts quick sort on the host list.
// Hosts with more open ports are placed earlier in the vector.
void Scanner::sortResults() {
    if (hosts.empty())
        return;
    
    quickSort(hosts, 0, hosts.size() - 1);
}

// Recursive quick sort implementation.
// This meets both the sort algorithm and recursion algorithm requirements.
void Scanner::quickSort(std::vector<Host>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quickSort(arr, low, pi -1);
        quickSort(arr, pi + 1, high);
    }
}

// Partition step for quick sort. The pivot is the open port count of the last host.
int Scanner::partition(std::vector<Host>& arr, int low, int high) {
    int pivot = arr[high].openPorts.size();
    int i = low - 1;

    for (int j = low; j < high; j++) {
        int current = arr[j].openPorts.size();

        if (current > pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }

    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Sorts a host's open ports by port number so binary search can work correctly.
void Scanner::sortPortsByNumber(Host& host) {
    std::sort(host.openPorts.begin(), host.openPorts.end(), [](const PortInfo& a, const PortInfo& b) {
        return a.port < b.port;
    });
}

// Binary search implementation to find a specific open port number.
int Scanner::binarySearchPort(const Host& host, int targetPort) {
    int low = 0;
    int high = static_cast<int>(host.openPorts.size()) - 1;

    while (low <= high) {
        // Calculates the midpoint safely to avoid overflow
        int mid = low + (high - low) / 2; 

        if (host.openPorts[mid].port == targetPort) {
            return mid; // Target found! Returns the index in the vector
        }
        
        if (host.openPorts[mid].port < targetPort) {
            low = mid + 1; // Search the right half
        } else {
            high = mid - 1; // Search the left half
        }
    }

    return -1; // Port not found in the open ports list
}
