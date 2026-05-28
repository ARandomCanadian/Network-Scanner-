#include <iostream>

#include "Scanner.h"
#include "ReportManager.h"

void start() {
    Scanner scanner;
    ReportManager reportManager;
 
    scanner.scanPorts();
}

void shutdown() {

}

int main (){
    start();
}