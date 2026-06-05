#include <iostream>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

#include "ReportManager.h"
#include "PortInfo.h"
#include "Host.h"

using json = nlohmann::json;

void ReportManager::saveToFile(const std::vector<Host>& hosts) {

    json root;

    for (const auto& host : hosts)
    {
        json hostJson;

        hostJson["ip"] = host.ip;
        hostJson["hostname"] = host.hostname;
        hostJson["online"] = host.online;

        for (const auto& port : host.openPorts)
        {
            json portJson;

            portJson["port"] = port.port;
            portJson["service"] = port.service;
            portJson["banner"] = port.banner;
            portJson["state"] = portStateToString(port.state);

            hostJson["openPorts"].push_back(portJson);
        }

        root["hosts"].push_back(hostJson);
    }

    std::ofstream outFile(fileName);

    if (!outFile.is_open())
    {
        std::cerr << "Failed to open file: " << fileName << '\n';
        return;
    }

    outFile << root.dump(4);
    outFile.close();

    std::cout << "Saved to " << fileName << '\n';
}

std::vector<Host> ReportManager::loadFromFile() {

}

std::string ReportManager::portStateToString(PortInfo::PortState state){
    switch (state){
        case PortInfo::PortState::OPEN: return "OPEN";
        case PortInfo::PortState::CLOSED: return "CLOSED";
        case PortInfo::PortState::FILTERED: return "FILTERED";
        case PortInfo::PortState::UNREACHABLE: return "UNREACHABLE";
        default: return "ERR";
    }
}

PortInfo::PortState ReportManager::stringToPortState(const std::string& state)
{
    if (state == "OPEN")
        return PortInfo::PortState::OPEN;

    if (state == "CLOSED")
        return PortInfo::PortState::CLOSED;

    if (state == "FILTERED")
        return PortInfo::PortState::FILTERED;

    if (state == "UNREACHABLE")
        return PortInfo::PortState::UNREACHABLE;

    return PortInfo::PortState::ERR;
}