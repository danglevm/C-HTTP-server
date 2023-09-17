#include <asm-generic/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>
#include <stddef.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#define PORT "8080" //the port the client will be connecting to
#define MAX_DATA_SIZE 128//number of data bytes that can be obtained
/**
variableNames in camelCase
functionNames() in camelCase
#define MACRO_NAME in CAPITAL_SNAKE_CASE
file_names in snake_case
type_defs in snake_case with _t suffix 
*
*/


//typecast sockaddr to sockaddr_in for IPv4 or sockaddr_in6 for ipv6
void * get_in_addr (struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        //return a pointer to an struct containing internet address for IPv4
        return &(((struct sockaddr_in*)addr)->sin_addr);
    }

    //IPv6 - AF_INET6
    return &(((struct sockaddr_in6 *)addr)->sin6_addr);

}


int main() {
    struct addrinfo hints;
    struct addrinfo *servInfo, *head;
    int sockfd; //server socket, not client
    int enableOption = 1;

    char request [] = "GET / HTTP/1.0 \r\n\r\n";

    //buffer - inet
    char dst[INET6_ADDRSTRLEN];
    
    //buffer - recv
    char recv_buff [MAX_DATA_SIZE];

    //set up criteria for addrinfo selection
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;

    int errCode = getaddrinfo(NULL, PORT, &hints, &servInfo);
    if (errCode != 0) {
        fprintf(stderr, "Client: get address info error: %s\n", gai_strerror(errCode));
        return EXIT_FAILURE;
    }

    for (head = servInfo; head != NULL; head = head->ai_next){
        sockfd = socket(head->ai_family, head->ai_socktype, head->ai_protocol);

        if (sockfd == -1) {
            perror("Client: unable to create socket error. Moving onto next address...");
            continue;
        }

        if (connect(sockfd, head->ai_addr, head->ai_addrlen) == -1){
            fprintf(stderr, "Client: unable to connect to webserver. Error code: %i. Moving onto next address...", errno);
            close(sockfd);
            continue;
        }

        //found the suitable struct or linked list is completely null
        break;

    }

    if (head == NULL) {
        perror ("Client: connection cannot be established with webserver.");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Connecting to the server...\n");
    inet_ntop(head->ai_family, get_in_addr((struct sockaddr *)head->ai_addr), dst, sizeof(dst));
    printf ("Client: connected to %s\n", dst);

    freeaddrinfo(servInfo);

     if (send(sockfd, request, sizeof request, 0) == -1) {
            perror("Client: send request error. ");
            close(sockfd);
            return EXIT_FAILURE;
        }


    //receiving from socket
    int numBytes = recv(sockfd, recv_buff, sizeof(recv_buff), 0);
    if (numBytes == -1){
        perror ("Client: failed trying to receive message from server");
        return EXIT_FAILURE;
    }
    
    //resize the buffer to number of bytes received from socket and add null character to make null-terminated string
    recv_buff[numBytes] = '\0';

    printf("client: received \n'%s'\n", recv_buff);

    
    close(sockfd);

    return EXIT_SUCCESS;
}