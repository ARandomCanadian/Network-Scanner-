#pragma once

#include <iostream>

class PortResult{
    private:
        int port;
        bool isOpen;
        std::string service;

    private:
        std::string ToString();
};