#include "socketutil.h" 

int createTCPIPv4Socket() { 
    return socket(AF_INET, SOCK_STREAM, 0); 
}

struct sockaddr_in* createIPv4Address(std::string ip, int port)
{
    struct sockaddr_in *address = new sockaddr_in;
    if (address == NULL) {
        // Handle allocation failure
        return NULL;
    }
    
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    
    if(ip.length() == 0)
        // For server sockets
        address->sin_addr.s_addr = INADDR_ANY;
    else
        // Clients
        inet_pton(AF_INET, ip.c_str(), &address->sin_addr.s_addr);

    return address;
}
