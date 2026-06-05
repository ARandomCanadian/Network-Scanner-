#include <iostream>
#include <winsock2.h>
#include <string>
#include <vector>

#include "Scanner.h"
#include "ReportManager.h"
#include "Host.h"
#include "PortInfo.h"

// Cleans up Winsock before the program exits.
// This is required because WSAStartup is called at the beginning of main.
void shutdownWinsock() {
    WSACleanup();
}

// Prints command-line help for the normal console mode and GUI scan mode.
void printUsage() {
    std::cout << "NetworkScanner.exe usage:\n";
    std::cout << "  NetworkScanner.exe                         Starts the normal console menu\n";
    std::cout << "  NetworkScanner.exe --gui-scan <subnet>      Scans network, scans ports, sorts, saves JSON\n";
    std::cout << "Example:\n";
    std::cout << "  NetworkScanner.exe --gui-scan 192.168.56.\n";
}

// Displays all currently stored hosts and their open ports in the console.
void printNetworkInformation(const std::vector<Host>& hosts) {
    std::cout << "Host Name | IP | Online\n";

    for (const Host& host : hosts) {
        std::cout << host.hostname << " | " << host.ip << " | " << host.online << "\n";

        if (!host.openPorts.empty()) {
            std::cout << "Port Number | Service | State | Banner\n";

            for (const PortInfo& port : host.openPorts) {
                std::cout << port.port << " | " << port.service << " | "
                          << static_cast<int>(port.state) << " | " << port.banner << "\n";
            }
        }
    }
}

// Runs the scanner without the console menu so the C# GUI can call this EXE.
// The GUI reads the JSON report that this function saves.
int runGuiScanMode(const std::string& subnet) {
    Scanner scanner;
    ReportManager reportManager;

    std::cout << "GUI scan mode started.\n";
    std::cout << "Scanning subnet: " << subnet << "\n";

    scanner.discoverHostsThreaded(subnet);

    std::cout << "Scanning ports 1-1024 on online hosts.\n";
    for (Host& host : scanner.getHost()) {
        std::cout << "Scanning ports on " << host.ip << "\n";
        scanner.scanPorts(host);
    }

    scanner.sortResults();
    reportManager.saveToFile(scanner.getHost());

    std::cout << "GUI scan mode complete. Results saved to scan_results.json\n";
    return 0;
}

// Program entry point. Handles command-line mode first, then falls back to the menu.
int main(int argc, char* argv[]) {
    WSADATA wsaData;

    // Initializes Winsock so sockets and ICMP networking functions can be used.
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return 1;
    }

    // If arguments were passed in, check whether the GUI is asking for a scan.
    if (argc > 1) {
        std::string mode = argv[1];

        if (mode == "--help" || mode == "-h") {
            printUsage();
            shutdownWinsock();
            return 0;
        }

        if (mode == "--gui-scan") {
            if (argc < 3) {
                std::cerr << "Missing subnet. Example: NetworkScanner.exe --gui-scan 192.168.56.\n";
                shutdownWinsock();
                return 1;
            }

            int result = runGuiScanMode(argv[2]);
            shutdownWinsock();
            return result;
        }

        std::cerr << "Unknown command line option: " << mode << "\n";
        printUsage();
        shutdownWinsock();
        return 1;
    }

    Scanner scanner;
    ReportManager reportManager;

    int userIn;
    std::string subnet;

    // Normal console menu used when the program is run directly by the user.
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
                shutdownWinsock();
                return 0;

            case 1:
                std::cout << "Enter subnet\n";
                std::cin >> subnet;
                std::cout << "Starting scan. May take a while\n";

                // Finds online hosts in the chosen subnet.
                scanner.discoverHostsThreaded(subnet);
                break;

            case 2:
                std::cout << "Scanning ports 1-1024 on hosts. May take a while\n";

                // Scans each discovered host for open ports.
                for (Host& host : scanner.getHost()) {
                    scanner.scanPorts(host);
                }

                std::cout << "Port Scan Complete\n";
                break;

            case 3:
                std::cout << "Printing information\n";
                printNetworkInformation(scanner.getHost());
                break;

            case 4:
                std::cout << "Outputting data to file\n";
                reportManager.saveToFile(scanner.getHost());
                std::cout << "Data outputted\n";
                break;

            case 5:
                std::cout << "Loading from file\n";
                // Loads saved scan data back into the scanner object.
                scanner.getHost() = reportManager.loadFromFile();
                std::cout << "Loaded from file\n";
                break;

            case 6:
                std::cout << "Sorting by most open ports\n";
                // Uses recursive quick sort to order hosts by most open ports.
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

                // Finds the host object that matches the user-provided IP address.
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

                // Binary search only works correctly after the ports are sorted.
                scanner.sortPortsByNumber(*targetHost);
                int index = scanner.binarySearchPort(*targetHost, targetPort);

                if (index != -1) {
                    const PortInfo& foundPort = targetHost->openPorts[index];
                    std::cout << "\n[+] Port " << targetPort << " is OPEN on " << searchIp << "!\n";
                    std::cout << "    Service: " << foundPort.service << "\n";
                    std::cout << "    Banner:  " << (foundPort.banner.empty() ? "None" : foundPort.banner) << "\n\n";
                } else {
                    std::cout << "\n[-] Port " << targetPort << " is NOT open or untracked on " << searchIp << ".\n\n";
                }
                break;
            }

            default:
                std::cout << "Invalid option\n";
                break;
        }
    }
}
