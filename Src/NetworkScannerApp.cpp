#include <iostream>
#include <winsock2.h>

#include "Scanner.h"
#include "ReportManager.h"
#include "Host.h"
#include "PortInfo.h"

void shutdown() {
    WSACleanup();
}

int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return 1;
    }

    Scanner scanner;
    ReportManager reportManager;

    int userIn;
    std::string subnet; 

    while (true) {
        std::cout << "0: Exit\n";
        std::cout << "1: Scan Network\n";
        std::cout << "2: Scan Ports\n";
        std::cout << "3: Print Network Information\n";
        std::cout << "4: Output To JSON\n";
        std::cout << "5: Input From JSON\n";
        std::cout << "6: Sort By Most Open Ports\n";
        std::cout << "7: Binary Search for a Specific Port\n";

        std::cin >> userIn;

        switch (userIn)
        {
            case 0:
                shutdown();
                return 0;

            case 1:
                std::cout << "Enter subnet\n";
                std::cin >> subnet;
                std::cout << "Starting scan. May take a while\n";

                scanner.discoverHostsThreaded(subnet);
                break;

            case 2:
                std::cout << "Scaning ports 1-1024 on hosts. May take a while\n";

                for (Host& host : scanner.getHost()) {
                    scanner.scanPorts(host);
                }

                std::cout << "Port Scan Complete\n";
                break;

            case 3:
                std::cout << "Printing information\n";

                std::cout << "Host Name | IP | Online\n";

                for (const Host& host : scanner.getHost()) {
                    std::cout << host.hostname << " | " << host.ip << " | " << host.online << "\n";

                    if (!host.openPorts.empty()) {
                        std::cout << "Port Number | Service | State | Banner\n";

                        for (const PortInfo& port : host.openPorts) {
                            std::cout << port.port << " | " << port.service << " | " << static_cast<int>(port.state) << " | " << port.banner << "\n";
                        }
                    }
                }
                break;
            
            case 4:
                std::cout << "Outputting data to file\n";
                reportManager.saveToFile(scanner.getHost());
                std::cout << "Data outputted\n";
                break;

            case 5:
                std::cout << "Loading from file\n";
                scanner.getHost() = reportManager.loadFromFile();
                std::cout << "Loaded from file\n";
                break;
            
            case 6:
                std::cout << "Sorting by most open ports\n";
                scanner.sortResults();
                std::cout << "Sorted\n";
                break;

            case 7: {
                if (scanner.getHost().empty()) {
                    std::cout << "No hosts scanned yet. Please scan the network and ports first.\n";
                    break;
                }
            
                std::string searchIp;
                int targetPort;
            
                std::cout << "Enter the IP address of the host to search within: ";
                std::cin >> searchIp;
                std::cout << "Enter the port number you are looking for: ";
                std::cin >> targetPort;
            
                // Find the host matching the entered IP address
                Host* targetHost = nullptr;
                for (Host& host : scanner.getHost()) {
                    if (host.ip == searchIp) {
                        targetHost = &host;
                        break;
                    }
                }
            
                if (targetHost == nullptr) {
                    std::cout << "Host with IP " << searchIp << " not found in results.\n";
                    break;
                }
            
                // 1. Sort the ports first (Required for Binary Search)
                scanner.sortPortsByNumber(*targetHost);
            
                // 2. Execute Binary Search
                int index = scanner.binarySearchPort(*targetHost, targetPort);
            
                // 3. Output results
                if (index != -1) {
                    const PortInfo& foundPort = targetHost->openPorts[index];
                    std::cout << "\n[+] Port " << targetPort << " is OPEN on " << searchIp << "!\n";
                    std::cout << "    Service: " << foundPort.service << "\n";
                    std::cout << "    Banner:  " << (foundPort.banner.empty() ? "None" : foundPort.banner) << "\n\n";
                } else {
                    std::cout << "\n[-] Port " << targetPort << " is NOT open (or untracked) on " << searchIp << ".\n\n";
                }
                break;
            }
            
            default:
                std::cout << "Invalid option\n";
                break;
        }
    }
}