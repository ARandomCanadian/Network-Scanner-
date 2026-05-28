#pragma once

#include <iostream>
#include <fstream>

class ReportManager{
    private:
        std::string fileName;

    public:
        void saveToFile();
        void loadFromFile();
        void exportReport();
};