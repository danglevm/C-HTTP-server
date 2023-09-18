#include <asm-generic/socket.h> //a bunch of socket options
#include <stddef.h> //NULL and size_t, ptrdiff_t
#include <strings.h>
#include <unistd.h>//for closing file socket. POSIX system?
#include <sys/types.h> //definitions of a number of data types used in syscalls
//for socket.h and in.h to work, need types.h
#include <sys/socket.h> //definitions of structures needed for sockets - sockaddr
#include <netdb.h> //for ai flags - AI_PASSIVE. Network database operations
#include <stdio.h> //C I/O inputs
#include <stdlib.h> //4 variable types, several macros, various functions
#include <arpa/inet.h> //definitions for internet operations - in_addr and in6_addr
#include <netinet/in.h>//defines some address info protocols
#include <string.h> //for string operations
#include <dirent.h> //for opening directories
#include <errno.h> //for error numbers

//could not get printf to print to stdout, only to stderr
#define PORT "8080" //port users will connect to. Alternative to port 80

#define BACKLOG_SIZE 15 //number of connections that can be in the queue.

#define MAX_DATA_SIZE 128 //max number of bytes

#define GET_REQUEST "GET"

#define HEAD_REQUEST "HEAD"

#define POST_REQUEST "POST"

#define RESPONSE_SIZE 1024 //1024 characters


//function for getting file extension
//since i have no understanding of regex for now, I assume that there are no folders with a '.' in it.
//I'll learn Regex and apply a search
const char *get_file_ext (const char *file_name) {
    const char * dot  = strrchr(file_name, '.');
    if (dot == NULL || dot == file_name) {
        return "";
    } 
    //return the character after the dot
    return dot + 1;

}

//function for getting file extension media type

const char *get_file_media_type (const char *file_ext){ 
    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "html") == 0) {
        return "text/html";
    } else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg")) {
        return "image/jpeg";
    } else if (strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    } else if (strcasecmp(file_ext, "txt") == 0){
        return "text/plain";
    } else if (strcasecmp(file_ext, "pdf") == 0) {
        return "application/pdf";
    } else {
        //for generic binary files or binary files missing extension
        return "aplication/octet-stream";
    }
}


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



//for building the http response, probably needs to be more flexible and detailed
char * buildHttpResponse (const char requestType [], const char requestPath[]) {
     //GET request
        if (strcmp(requestType, GET_REQUEST) == 0) {
            return     "HTTP/1.0 200 OK\r\n"
                       "Content-type: text/html\r\n\r\n"
                       "<html>Hello Goddamn World!</html>\r\n";
                
        }  else if ((strcmp(requestType, HEAD_REQUEST) == 0) || (strcmp(requestType, POST_REQUEST) == 0)){
            //HEAD or POST request
            return     "HTTP/1.0 501 Not Implemented\r\n"
                       "Content-type: text/html\r\n\r\n"
                       "<html>Not Hello World!</html>\r\n";
        } else {
            return     "HTTP/1.0 400 Bad Request\r\n"
                       "Content-type: text/html\r\n\r\n"
                       "<html>Goodbye World!</html>\r\n";
        }
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
    //one line separation between HTTP header and its body
    
    
    //storing the value of the socket file descriptor
    int sockfd;

    char recv_buff [MAX_DATA_SIZE];

    char response [RESPONSE_SIZE];
    

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

    //check if server directory can be opened
    DIR* dir = opendir("/home/bollabollo/Documents/C_Programs/dummy");
    if (dir == NULL) {
        perror ("Server: cannot open directory error.");
        return EXIT_FAILURE;
    } else {
        printf ("Server: successfully opened server directory\n");
    }

    //hostname = NULL will be assigned depending on the hints flag - 127.0.0.1?
    //hints - type of service requested
    status = getaddrinfo(NULL, PORT , &hints, &res);


    //gai_strerror translates error codes to a human readable string, suitable for error reporting
    if (status != 0) {
        fprintf (stderr, "Server: get address info error: %s\n", gai_strerror(status));
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
            perror("Server: unable to create socket error. Moving onto next address...");
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

    fprintf(stdout, "Waiting for connections...\n");

    //main loop for accepting connection
    for (;;) {
        int connected_sockfd;
        client_address_size = sizeof(client_address);
        char delim[] = " ";
        //client_address points to a sockaddr struct, filled in with the address of the peer socket
        connected_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_size);
        if (connected_sockfd == -1) {
            perror("Server: Client socket connection acceptance error. Moving on to next request...");
            continue;
        }

        

        //src (2nd parameter) can either point to a struct_inaddr or struct in6_addr, which is found by get_in_addr
        inet_ntop(client_address.ss_family, get_in_addr((struct sockaddr *)&client_address), dst, sizeof(dst));
        fprintf(stderr, "Server: received incoming connection from %s\n", dst); 
        
         //receiving from socket
        int numBytes = recv(connected_sockfd, recv_buff, sizeof(recv_buff), 0);
        if (numBytes == -1){
            perror ("Server: failed trying to receive message from server");
        
        }

        //get the request type
        char *token = strtok(recv_buff, delim);

        char *requestType = token;
        
        

        //get the URI path
        token = strtok(NULL, delim);
        char *URI = token;

        strcpy(response, buildHttpResponse(requestType, URI));
    
        if (send(connected_sockfd, response, strlen(response), 0) == -1) {
             perror("Server: send response error. ");
             close(connected_sockfd);
             return EXIT_FAILURE;
        }

        close (connected_sockfd);
    
    }
    return EXIT_SUCCESS;
}