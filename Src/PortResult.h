#pragma once

#include <iostream>

class PortResult{
    private:
        int port;
        bool isOpen;

        std::string service;
        std::string banner;

    private:
        std::string ToString();
};