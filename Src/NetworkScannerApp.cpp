#include <iostream>

#include "Scanner.h"
#include "ReportManager.h"

void start() {
    Scanner scanner;
    ReportManager reportManager;
    std::cout << "Project Started";
    
    scanner.recursiveScan(1);
}

void shutdown() {

}

int main (){
    start();
}