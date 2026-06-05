#include <iostream>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

#include "ReportManager.h"
#include "PortInfo.h"
#include "Host.h"

using json = nlohmann::json;

// Saves scan results to scan_results.json.
// This gives the project file output and lets the GUI load the scan results.
void ReportManager::saveToFile(const std::vector<Host>& hosts) {

    // Root JSON object that stores every discovered host.
    json root;
    root["hosts"] = json::array();

    for (const auto& host : hosts)
    {
        // Convert one Host object into a JSON object.
        json hostJson;

        hostJson["ip"] = host.ip;
        hostJson["hostname"] = host.hostname;
        hostJson["online"] = host.online;
        hostJson["openPorts"] = json::array();

        for (const auto& port : host.openPorts)
        {
            // Convert one open port into JSON data.
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

    outFile << root.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
    outFile.close();

    std::cout << "Saved to " << fileName << '\n';
}

// Loads scan_results.json and rebuilds the vector of Host objects.
// This gives the project file input and allows old scans to be viewed again.
std::vector<Host> ReportManager::loadFromFile()
{
    std::vector<Host> hosts;

    std::ifstream inFile(fileName);

    if (!inFile.is_open())
    {
        std::cerr << "Failed to open " << fileName << '\n';
        return hosts;
    }

    // Parse the JSON file into a JSON object.
    json root;
    inFile >> root;

    if (!root.contains("hosts"))
        return hosts;

    for (const auto& hostJson : root["hosts"])
    {
        std::string ip = hostJson["ip"];
        bool online = hostJson["online"];

        // Recreate the Host object from the saved JSON values.
        Host host(ip, online);

        if (hostJson.contains("hostname"))
        {
            host.hostname = hostJson["hostname"];
        }

        if (hostJson.contains("openPorts"))
        {
            for (const auto& portJson : hostJson["openPorts"])
            {
                int portNumber = portJson["port"];

                // Recreate the PortInfo object from the saved JSON values.
                PortInfo port(
                    stringToPortState(portJson["state"]),
                    portNumber
                );

                if (portJson.contains("service"))
                {
                    port.service = portJson["service"];
                }

                if (portJson.contains("banner"))
                {
                    port.banner = portJson["banner"];
                }

                host.openPorts.push_back(port);
            }
        }

        hosts.push_back(host);
    }

    return hosts;
}

// Converts the enum state into text so it can be stored in JSON.
std::string ReportManager::portStateToString(PortInfo::PortState state){
    switch (state){
        case PortInfo::PortState::OPEN: return "OPEN";
        case PortInfo::PortState::CLOSED: return "CLOSED";
        case PortInfo::PortState::FILTERED: return "FILTERED";
        case PortInfo::PortState::UNREACHABLE: return "UNREACHABLE";
        default: return "ERR";
    }
}

// Converts text from JSON back into the matching enum value.
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