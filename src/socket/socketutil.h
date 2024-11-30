#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

int createTCPIPv4Socket();
struct sockaddr_in* createIPv4Address(std::string, int port);

#endif

