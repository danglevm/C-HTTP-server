#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

//socket descriptor is of type int

//holds socket address info for many types of sockets
//avoid packing the address in sa_data by hand
typedef struct sockaddr {
    unsigned short sa_family; //address family - AF_INET/AF_INET6
    char           sa_data[14]; //destination address and port number for the socket
} sockaddr;


//prepares socket address structures for subsequent use
//call getaddrinfo() - return a pointer to a new linked list of these structures
typedef struct addrinfo{
    int     ai_flags;
    int     ai_family; //AF_NET (IPv4), AF_INET6 (IPv6), AF_UNSPEC (use whatever), address family
    int     ai_socktype; //Stream socket or datagram; SOCK_STREAM, SOCK_DGRAM
    int     ai_protocol; //use "0" for any
    size_t  ai_addrlen; //size of ai_addr in bytes
    sockaddr *ai_addr;
    char    *ai_canonname; //full canonical hostname

    struct addrinfo *ai_next; //linked list next node
} addrinfo;




int main() {
    return EXIT_SUCCESS;
}