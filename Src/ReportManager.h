#pragma once

#include <iostream>
#include <vector>

#include "PortInfo.h"
#include "Host.h"

class ReportManager{
    private:
        std::string fileName = "scan_results.json";

    public:
        void saveToFile(const std::vector<Host>& hosts);
        std::vector<Host> loadFromFile();
        std::string portStateToString(PortInfo::PortState state);
        PortInfo::PortState ReportManager::stringToPortState(const std::string& state);
};