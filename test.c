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



/*
* A pointer to a struct sockaddr_in4 can be cast to a pointer to a struct sockaddr and vice versa
* connect() wants a pointer to struct sockaddr but you can use struct sockadder_in4 and cast it at the last minute
*/

typedef struct in4_addr {
    uint32_t s_addr; //32-bit int
} in4_addr;

//for IPv4 connection
typedef struct sockaddr_in4 {
    short int          sin4_family; //address family - AF_INET/AF_INET6
    //short int occupies 2 bytes of memory, from -32,767 to 32,767
    unsigned short int sin4_port; //port number. Must be in the Network byte Order (by using htons()!)
    in4_addr    sin4_addr; //internet address
    unsigned char      sin_zero[8]; //same size as struct sockadder
    //sin_zero is included to pad the struct to the length of a struct sockaddr. 
    //sin_zero should be set to all zeros with the function memset()
} sockaddr_in4;




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


//sockaddr_storage. check the ss_family field to see if it's AF_INET or AF_INET6

int main() {
    return EXIT_SUCCESS;
}