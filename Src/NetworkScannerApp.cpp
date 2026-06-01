#include <iostream>

#include "Scanner.h"
#include "ReportManager.h"

void start() {
    Scanner scanner;
    ReportManager reportManager;
    std::cout << "Project Started";
    
    scanner.thrededScan();
}

void shutdown() {

}

int main (){
    start();
}