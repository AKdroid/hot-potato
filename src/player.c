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
#include"comm.h"


void print_usage(){
    
    printf("player <master-machine-name> <port-number>\n");

}

int main(int argc, char* argv[]){

    int identifier, master_port_num;
    char* hostname;
    char readbuffer[1024];
    char writebuffer[1024];
    struct sockaddr_in* master=NULL, *left=NULL, *right=NULL;
    int master_sock, left_sock, right_sock;


    if(argc!=3){
        print_usage();
        exit(1);
    }

    hostname = argv[1];

    master_port_num = atoi(argv[2]);

    master = connect_to_master(hostname,master_port_num,&master_sock);

    if(master == NULL){
        printf("Connection Failed\n");
        exit(1);  
    }

    printf("Connection successful\n");

    close(master_sock);

    return 0;
}
