#include <iostream>
#include <map>

class ServiceDetection{
    private:
        std::map<int, std::string> knownServices;

    public: 
        ServiceDetection();

        std::string getServiceName(int port);
};