#include <iostream>
#include <map>

class ServiceDetection{
    private:
        std::map <int, std::string> knownService;

    public: 
        std::string detectService();
        void loadService();
};