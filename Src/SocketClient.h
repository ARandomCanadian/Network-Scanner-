#pragma once

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <cstdint>

class SocketClient{
    private:

    public:
        bool connectToPort(std::string ip, uint16_t port);
        void closeConnection();
};