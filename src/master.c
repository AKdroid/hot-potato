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

struct player{
    int sock;
    struct sockaddr_in saddr;
    int len;
    int leftport;
    int rightport;
    char hostname[64];
};

typedef struct player player;

void print_usage(){
    printf("Usage: master <port-number> <number-of-players> <hops>\n");
}

int main(int argc, char* argv[]){

    long num_of_players, port_num, hops;
    int state;
    int socket_id,client;
    int registered_count = 0;
    player* players;

    struct sockaddr_in* listener=NULL;

    if(argc != 4 ){
        print_usage();
        exit(1);
    }
        
    port_num = atoi(argv[1]);
    num_of_players = atoi(argv[2]);
    hops = atoi(argv[3]);

    players = (player*) malloc(num_of_players * sizeof(player));

    listener = initialize_master(port_num,&socket_id);
    
    if(listener == NULL || num_of_players <= 0){
        printf("Master could not be started\n");
        free(players);
        exit(1);
    }



    while(registered_count < num_of_players){

        listen(socket_id,10);

        players[registered_count].sock = accept(socket_id,(struct sockaddr*)&(players[registered_count].saddr),&(players[registered_count].len));
        if(players[registered_count].sock < 0){
            perror("Accept: ");
        }

        allocate_id(players[registered_count].sock,registered_count,&(players[registered_count].hostname));

        registered_count++;    
        

    }
    

    return 0;
}
