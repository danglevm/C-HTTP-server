#include <asm-generic/socket.h>
#include <stddef.h>
#include <unistd.h>//for closing file socket. POSIX system?
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

//could not get printf to print to stdout, only to stderr
#define PORT "8080" //port users will connect to. Alternative to port 80

#define BACKLOG_SIZE 15 //number of connections that can be in the queue.

#define BUFFER_SIZE 4096



//typecast sockaddr to sockaddr_in for IPv4 or sockaddr_in6 for ipv6
//
void * get_in_addr (struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        //return a pointer to an struct containing internet address for IPv4
        return &(((struct sockaddr_in*)addr)->sin_addr);
    }

    //IPv6 - AF_INET6
    return &(((struct sockaddr_in6 *)addr)->sin6_addr);

}

// void build200HTTPResponse (char *response, size_t response_len){ 
//     //reason phrase OK 
//     //carriage character \r
//     //
//     char *message = "HTTP/1.0 200 OK \r\n";
//     snprintf(message, sizeof(message), message, ...)
// }




//port number specified as a command line argument
//argc - number of paramaters plus 1 to include the name of the program executed to get the process running
//argc - must always be larger than 0
//arg count, arg[0] - name of executable, arg[1] - port in this case
int main (int argc, char *argv []) {
    int status;

    char response [] = "HTTP/1.0 200 OK\r\n"
                       "Content-type: text/html\r\n\r\n"
                       "<html>Hello World!</html>\r\n";

    size_t response_len = sizeof(response);

    //storing the value of the socket file descriptor
    int sockfd, connected_socketfd;


    /**
    *
    */
    //hints points to an addrinfo struct that specifies criteria for the 
    //socket address structures returned in the list pointed by res
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *head;
    //length of IPv6 address string, which is longer than IPv4, for use by inet_ntop
    char dst [INET6_ADDRSTRLEN];

    int enable_sockopt = 1;

    //only contains the client's address, which is what we care about
    struct sockaddr_storage client_address;
    socklen_t client_address_size;

    /*
    *
    * for use by getpeername()
    */
    struct sockaddr * peer;
    

    // //1 for the port, 1 for the program
    // if (argc != 2) {
    //     perror("Port argument unspecified");
    //     return EXIT_FAILURE; //return 1 in main means program does not execute successfully and there is some error
    // }

    //void *, int, unsigned long - set n characters of string to c for n numbers
    //reset hints, which is resetting the criteria for the service returned by getaddrinfo
    memset(&hints, 0, sizeof hints);

    //since hints is non-NULL, it points to an addrinfo struct with criteria that limit the set of socket addresses returned by getaddrinfo()
    hints.ai_family = AF_UNSPEC; //AF-agnostic - IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; //Stream socket - bidirectional
    hints.ai_protocol = 0; //protocol for the returned socket address
    hints.ai_flags = AI_PASSIVE; //Assign the address of the local host to the socket structure


    //hostname = NULL will be assigned depending on the hints flag - 127.0.0.1?
    //hints - type of service requested
    status = getaddrinfo(NULL, PORT , &hints, &res);

    
    //gai_strerror translates error codes to a human readable string, suitable for error reporting
    if (status != 0) {
        fprintf (stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    
    
    //looping through the linked list
    //ai_next - points to the next node addrinfo pointer
    for (head = res; head != NULL; head = head->ai_next) {
        //tries to find a valid struct and bind to it
        
        //Domain should be IPv4 or IPv6
        //Socket type should be sock_stream
        sockfd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);
        
        if (sockfd == -1){ //error
        //perror - print to stderror stream
            perror("Server: socket file descriptor error. Moving onto next address...");
            continue;
        }
        /**
        * socket file descriptor
        * at the socket level: SOL_SOCKET
        * SO_REUSEADDR - reuse of local address
        * to enable sockopt, value of sockopt must be non-zero
        */
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable_sockopt, sizeof(enable_sockopt)) == -1){
            perror("Server: socket option error. Moving onto next address...");
            continue;
        }

        if ((bind(sockfd, head->ai_addr, head->ai_addrlen)) == -1) {
            perror("Server: socket binding error. Moving onto next address...");
            close(sockfd);
            continue;
        }


        //once we found the valid struct, stop finding
        break;
    }
    //free one or more addrinfo structures returned from getaddrinfo
    freeaddrinfo(res);


    //if we go through the whole thing, and there is no valid struct
    if (head == NULL) {
        perror("Server: failed to bind to a socket error. Shutting down the server...");
        return EXIT_FAILURE;
    }
    //listen on connection socket
    if (listen(sockfd, BACKLOG_SIZE) == -1) {
        perror("Server: error listening on socket. Shutting down the server...");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Waiting for connections...\n");

    //main loop for accepting connection
    for (;;) {
        client_address_size = sizeof(client_address);
        //client_address points to a sockaddr struct, filled in with the address of the peer socket
        connected_socketfd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_size);
        if (connected_socketfd == -1) {
            perror("Server: Client socket connection acceptance error. Moving on to next request...");
            continue;
        }

        

        //src (2nd parameter) can either point to a struct_inaddr or struct in6_addr, which is found by get_in_addr
        inet_ntop(client_address.ss_family, get_in_addr((struct sockaddr *)&client_address), dst, sizeof(dst));
        fprintf(stderr, "Server: received incoming connection from %s\n", dst); 
        
            
        
        if (send(connected_socketfd, response, response_len, 0) == -1) {
            perror("Server: message send error. ");
            close(connected_socketfd);
            return EXIT_FAILURE;
        }

        close (connected_socketfd);
    
    }
    return EXIT_SUCCESS;
}