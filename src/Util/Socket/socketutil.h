#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <string>

#ifdef _WIN32
    #include <WinSock2.h>
    #include <ws2tcpip.h>
#elif __linux__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#elif _MACOS
#endif

#include <stdlib.h>

int createTCPIPv4Socket();
struct sockaddr_in* createIPv4Address(std::string, int port);

#endif

