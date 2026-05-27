#pragma once

#include <iostream>
#include <vector>

#include "Host.h"
#include "PortResult.h"

class Scanner {
    private:
        std::string targetIP;
        int startPort;
        int endPort;
        std::vector<Host> hosts;
    
    public:
        void scanPorts(){

        }

        bool pingHost(){

        }

        void sortResults(){

        }

        PortResult searchPort(int port){
            
        }

        void recursiveScan(int port){

        }
};