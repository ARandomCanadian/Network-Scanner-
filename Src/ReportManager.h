#pragma once

#include <iostream>
#include <vector>

#include "PortInfo.h"
#include "Host.h"

// Handles file input and output for scan results.
// The GUI depends on this class because it reads the JSON file created here.
class ReportManager{
    private:
        // Default report file shared by the console app and the GUI app.
        std::string fileName = "scan_results.json";

    public:
        // Saves all discovered hosts and their open ports to a JSON report.
        void saveToFile(const std::vector<Host>& hosts);

        // Loads hosts and ports back from the JSON report.
        std::vector<Host> loadFromFile();

        // Converts a PortState enum into readable text for JSON output.
        std::string portStateToString(PortInfo::PortState state);

        // Converts saved JSON text back into a PortState enum.
        PortInfo::PortState stringToPortState(const std::string& state);
};
