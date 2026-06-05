#pragma once

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <cstdint>

#include "PortInfo.h"

// Handles low-level TCP connection attempts for the scanner.
class SocketClient{
    private:
        // No saved socket is needed because each port scan creates and closes its own socket.

    public:
        // Attempts to connect to an IP and port, then returns OPEN, CLOSED, FILTERED, etc.
        PortInfo::PortState connectToPort(std::string ip, uint16_t port);

        // Kept for possible future cleanup logic if persistent sockets are added later.
        void closeConnection();
};
