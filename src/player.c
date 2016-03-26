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

void playgame(){

    int x;
    while(1){
        scanf("%d",&x);
        if(x==0)
            break;
    }
}

int main(int argc, char* argv[]){

    int identifier, master_port_num, l_id, r_id;
    char* hostname;
    char readbuffer[1024];
    char writebuffer[1024];
    struct sockaddr_in* master=NULL, *left=NULL, *right=NULL;
    int master_sock, left_sock, right_sock, hops, reader_sock;
    char *body, *readbuf, *bodynull = "";
    
    int sockets[3];


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

    register_client(master_sock,&identifier, &l_id, &r_id);

    printf("Connected as player %d\n",identifier);

    connect_to_neighbor(master_sock, &left, &right, &left_sock, &right_sock);

    connect_to_neighbor(master_sock, &left, &right, &left_sock, &right_sock);

    srand((unsigned) identifier);

    sockets[0]=master_sock;
    sockets[1]=left_sock;
    sockets[2]=right_sock;

    while(1){

        reader_sock=select_readable_socket(sockets,3,3600);        
        if(reader_sock < 0)
            break;
        body=NULL;
        get_potato(reader_sock, &hops, &readbuf, &body);
        if(hops < 0)
            break;
        if(hops > 1){
            reader_sock = rand()%2 + 1;
            printf("Sending potato to %d\n",reader_sock==1?l_id:r_id);
        }
        else{
            reader_sock = 0;
            printf("I'm it\n");
        }
        if(body == NULL)
            pass_potato(sockets[reader_sock], identifier, hops, 0, bodynull);
        else
            pass_potato(sockets[reader_sock], identifier, hops, 1, body);

        free(readbuf);
    }

    free(readbuf);
    shutdown(left_sock,SHUT_RDWR);
    shutdown(right_sock,SHUT_RDWR);
    shutdown(master_sock,SHUT_RDWR);
    close(left_sock);
    close(right_sock);
    close(master_sock);

    return 0;
}
