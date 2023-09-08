#include <sys/types.h> //definitions of a number of data types used in syscalls
//for socket.h and in.h to work, need types.h
#include <sys/socket.h> //definitions of structures needed for sockets - sockaddr
#include <netdb.h> //for ai flags - AI_PASSIVE
#include <stdio.h> //C I/O inputs
#include <stdlib.h> //4 variable types, several macros, various functions
#include <netdb.h> //definitions for network db operations
#include <arpa/inet.h> //definitions for internet operations - in_addr and in6_addr
#include <netinet/in.h>//defines some address info protocols
#include <string.h> //for string operations


#define PORT "3490" //port users will connect to

//port number specified as a command line argument
//argc - number of paramaters plus 1 to include the name of the program executed to get the process running
//argc - must always be larger than 0
//arg count, arg[0] - name of executable, arg[1] - port in this case
int main (int argc, char *argv []) {
    int status;

    //hints points to an addrinfo struct that specifies criteria for the 
    //socket address structures returned in the list pointed by res
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *head;
    //length of IPv6 address string
    char ipstr [INET6_ADDRSTRLEN];

    //1 for the port, 1 for the program
    if (argc != 2) {
        fprintf(stderr,"Error - port argument unspecified");
        return 1; //return 1 in main means program does not execute successfully and there is some error
    }

    //void *, int, unsigned long - set n characters of string to c for n numbers
    memset(&hints, 0, sizeof hints);

    //since hints is non-NULL, it points to an addrinfo struct with criteria that limit the set of socket addresses returned by getaddrinfo()
    hints.ai_family = AF_UNSPEC; //AF-agnostic - IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; //Stream socket - bidirectional
    hints.ai_protocol = 0; //protocol for the returned socket address
    hints.ai_flags = AI_PASSIVE; //Assign the address of the local host to the socket structure

    //getaddressinfo(hostname, port, hints, results)
    //hostname = NULL will be assigned depending on the hints flag - 127.0.0.1?
    //hints - type of service requested
    status = getaddrinfo(NULL, argv[1] , &hints, &res);

    //0 - success, non-zero - error. 
    //gai_strerror translates error codes to a human readable string, suitable for error reporting
    if (status != 0) {
        fprintf (stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    
    int sockfd;
    //looping through the linked list
    //ai_next - points to the next node addrinfo pointer
    for (head = res; head != NULL; head = head->ai_next) {
        //tries to find a valid struct and bind to it
        
        sockfd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);
        
        if (sockfd == -1){ //error
        //perror - print to stderror stream
            perror("Server: socket file descriptor error. Moving onto next address...");
            continue;
        }

        if (bind(sockfd, head->ai_addr, head->ai_addrlen)) {
            perror("Server: socket binding error. Moving onto next address...");
            continue;
        }


        //once we found the valid struct, stop finding
        break;
    }
    //free one or more addrinfo structures returned from getaddrinfo
    freeaddrinfo(res);
    return EXIT_SUCCESS;
}