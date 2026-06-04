#include <iostream>
#include <map>

class ServiceDetection{
    private:
        std::map<int, std::string> knownServices;

    public: 
        ServiceDetection();
        std::string grabBanner(const std::string& ip, int port);
        std::string detectService(const std::string banner, int port);
};