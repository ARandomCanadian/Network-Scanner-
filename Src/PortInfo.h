#pragma once

#include <iostream>

// Stores the result of scanning one port on one host.
struct PortInfo
{
    // Possible states returned by the socket connection attempt.
    enum class PortState {
        OPEN,
        CLOSED,
        FILTERED,
        UNREACHABLE,
        ERR
    };

    // Port number that was scanned, for example 22, 80, or 443.
    int port;

    // Service name detected from the banner or common port number.
    std::string service;

    // Text returned by the service, if the server sends a banner.
    std::string banner;

    // Final scan result for this port.
    PortState state;

    // Constructor used when a port scan returns a state and port number.
    PortInfo(PortState portState, int port) :  port(port), state(portState) {};
};
