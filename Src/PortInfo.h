#include <iostream>

struct PortInfo
{
    enum class PortState {
        OPEN,
        CLOSED,
        FILTERED,
        UNREACHABLE,
        ERR
    };

    int port;
    std::string service;
    PortState state;

    PortInfo(PortState portState, int port) : state(portState), port(port) {};
};
