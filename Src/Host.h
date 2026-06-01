#pragma once

#include <iostream>
#include <vector>

#include "PortInfo.h"

struct Host {
    std::string ip;
    std::string hostname;
    bool online;
    std::vector<PortInfo> openports;
    
    Host(const std::string& ipAddress, bool isOnline) : ip(ipAddress), online(isOnline) {}
};