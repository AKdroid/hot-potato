#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/fcntl.h>
#include<sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct sockaddr_in* initialize_master(int portnum, int* s){
    
    struct sockaddr_in* master_socket=NULL;
    struct hostent *hp=NULL;
    char hostname[64];
    *s = socket(AF_INET, SOCK_STREAM, 0);
    
    if(*s < 0){
        perror("Socket:");
        return NULL;
    }

    gethostname(hostname, sizeof hostname);
    hp = gethostbyname(hostname);

    if(hp == NULL){
        perror("gethostbyname:");
        return NULL;
    }

    master_socket = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));
    
    if(master_socket == NULL){
        perror("malloc:");
        return NULL;
    }

    master_socket->sin_family = AF_INET; 
    master_socket->sin_port = htons(portnum);

    memcpy(&master_socket->sin_addr,hp->h_addr_list[0],hp->h_length);

    bind(*s, (struct sockaddr*) master_socket, sizeof(*master_socket));
    
    return master_socket;
}

struct sockaddr_in* connect_to_master(char* hostname, int portnum, int* s){

    struct sockaddr_in* master_socket=NULL;
    struct hostent *hp=NULL;
    int rc;
    *s = socket(AF_INET, SOCK_STREAM, 0);

    if(*s < 0){
        perror("Socket:");
        return NULL;
    }
    
    hp = gethostbyname(hostname);
    
    if(hp == NULL){
        perror("gethostbyname:");
        return NULL;
    }

    master_socket = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));

    if(master_socket == NULL){
        perror("malloc:");
        return NULL;
    }

    master_socket->sin_family = AF_INET;
    master_socket->sin_port = htons(portnum);  

    memcpy(&master_socket->sin_addr,hp->h_addr_list[0],hp->h_length);

    rc = connect(*s, (struct sockaddr *)master_socket, sizeof(*master_socket));

    if(rc < 0){
        perror("Connect: ");
        return NULL;
    }

    return master_socket;

}
