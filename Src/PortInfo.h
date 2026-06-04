#pragma once

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
    std::string banner;
    PortState state;

    PortInfo(PortState portState, int port) :  port(port), state(portState) {};
};
