#pragma once

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <cstdint>

#include "PortInfo.h"

class SocketClient{
    private:

    public:
        PortInfo::PortState connectToPort(std::string ip, uint16_t port);
        void closeConnection();
};