#pragma once

#include <iostream>
#include <map>

// Detects service information for open ports.
// It can grab banners and use common port numbers as a fallback.
class ServiceDetection{
    private:
        // Reserved for future use if known services are loaded into a map.
        std::map<int, std::string> knownServices;

    public: 
        // Sets up the service detection object.
        ServiceDetection();

        // Connects to a port and attempts to read text sent by the service.
        std::string grabBanner(const std::string& ip, int port);

        // Uses banner text and port number to guess the service name.
        std::string detectService(const std::string banner, int port);
};
