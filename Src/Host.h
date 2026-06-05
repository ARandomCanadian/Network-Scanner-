#pragma once

#include <iostream>
#include <vector>

#include "PortInfo.h"

// Stores all scan information for one device found on the network.
// This is a struct because it mainly groups related data together.
struct Host {
    // IP address of the device, for example "192.168.56.100".
    std::string ip;

    // Optional host name. This can stay empty if no name is found.
    std::string hostname;

    // True when the host responds to the network discovery ping.
    bool online;

    // List of ports that were found open on this host.
    std::vector<PortInfo> openPorts;
    
    // Constructor used when a new host is discovered.
    Host(const std::string& ipAddress, bool isOnline) : ip(ipAddress), online(isOnline) {}
};
