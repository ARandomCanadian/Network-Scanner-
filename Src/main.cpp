#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main (){
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed with error " << result << endl;
        return 1;
    }

    WSACleanup();
    return 0;
}