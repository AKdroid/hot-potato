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

void register_client(int master,int *id){
    
    char sendbuffer[1024];
    char readbuffer[1024];
    char hostname[64],temp[13];
    char*x,*y;
    int sent,rcvd;    

    gethostname(hostname,sizeof(hostname));
    
    sendbuffer[0]=0;

    strcat(sendbuffer,"#REGISTER# ");
    strcat(sendbuffer,hostname);
    strcat(sendbuffer," #END#");
    
    sent = send(master,sendbuffer,strlen(sendbuffer),0);

    if(sent < 0){
        perror("Sent:");
        return;
    }
    
    rcvd=recv(master,readbuffer,1024,0);
    if(rcvd > 0){
        readbuffer[rcvd]=0;
        y = strstr(readbuffer,"#END#");
        if(y!=NULL){
            x = strstr(readbuffer,"#SETID#");
            if(x != NULL){
                strncpy(temp,readbuffer+8,y-readbuffer-8);
                *id = atoi(temp);
            }
        }
    }
    if(rcvd < 0){
        perror("read");
    }

    sendbuffer[0]=0;
    strcat(sendbuffer,"#ACK# 0 #END#");
    
    sent = send(master,sendbuffer,13,0);

    if(sent < 0){
        perror("Sent:");
        return;
    }

}

void allocate_id(int client_socket, int id, char* hostname){
    
    char sendbuffer[1024];
    char readbuffer[1024];
    char temp[13];
    int sent,rcvd;
    char* x;


    rcvd=recv(client_socket,readbuffer,1024,0);
    if(rcvd > 0){
        readbuffer[rcvd]=0;
        x = strstr(readbuffer," #END#");
        strncpy(hostname,readbuffer+11,x-readbuffer-11);
        printf("Hostname of client = %s\n",hostname);
    }
    if(rcvd < 0){
        perror("read");
    }

    sendbuffer[0]=0;

    snprintf(temp,12,"%d",id);

    strcat(sendbuffer,"#SETID# ");
    strcat(sendbuffer,temp);
    strcat(sendbuffer," #END#");
    
    sent = send(client_socket,sendbuffer,strlen(sendbuffer),0);

    if(sent < 0){
        perror("Sent:");
        return;
    }

    rcvd=recv(client_socket,readbuffer,1024,0);
    if(rcvd > 0){
        readbuffer[rcvd]=0;
        x=strstr(readbuffer,"#END#");
        if(x!=NULL){
            x=strstr(readbuffer,"#ACK#");
            if(x != readbuffer){
                return 0;
            }
        }
    }
    if(rcvd < 0){
        perror("read");
    }
    return 1;
}
