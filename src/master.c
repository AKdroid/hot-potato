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

void wait_for_players(int socket_id, player* players, int count, int MAX){

        int setflag=1;

        while(count < MAX){

        listen(socket_id,10);

        players[count].sock = accept(socket_id,(struct sockaddr*)&(players[count].saddr),&(players[count].len));

        if(players[count].sock < 0){
            perror("Accept: ");
        }

        setsockopt(players[count].sock,SOL_SOCKET,SO_KEEPALIVE,&setflag,1);

        allocate_id(players[count].sock,count,(count-1+MAX)%MAX,(count+1)%MAX,&(players[count].hostname));

        printf("player %d is on %s\n",count,players[count].hostname);

        count++;
    }
}

void build_ring(player* players, int MAX){
    
    int i;
    int li,co;
    

    for(i=0;i<MAX;i++){
        co = i; //right of i
        li = (i+1)%MAX; // left of i+1
        setup_left(players[li].sock,&(players[li].leftport));
        setup_right(players[co].sock,players[li].hostname,players[li].leftport);
    }

}

int main(int argc, char* argv[]){

    long num_of_players, port_num, hops, i;
    int state;
    int socket_id,client;
    int registered_count = 0;
    player* players;
    char hostname[64];
    
    int* player_sockets= NULL;

    struct sockaddr_in* listener=NULL;

    if(argc != 4 ){
        print_usage();
        exit(1);
    }
        
    port_num = atoi(argv[1]);
    num_of_players = atoi(argv[2]);
    hops = atoi(argv[3]);

    gethostname(hostname,64);

    printf("Potato Master on %s\n",hostname);
    printf("Players = %d\n",num_of_players);
    printf("Hops = %d\n",hops);

    players = (player*) malloc(num_of_players * sizeof(player));

    listener = initialize_master(port_num,&socket_id);
    
    if(listener == NULL || num_of_players <= 0){
        printf("Master could not be started\n");
        free(players);
        exit(1);
    }

    wait_for_players(socket_id, players, 0, num_of_players);

    registered_count = num_of_players; 

    build_ring(players, num_of_players);

    srand((unsigned) 1);

    client = rand()%num_of_players;
    
    printf("All players present, sending potato to player %d\n",client);

    player_sockets = (int*) malloc(num_of_players*sizeof(int));

    for(i=0;i<num_of_players;i++){
        player_sockets[i] = players[i].sock;
    }

    initiate_game(players[client].sock,hops);

    client = select_readable_socket(player_sockets, num_of_players, 3600);

    if(client <= 0){
        printf("Did not receive any trace back\n");
    }else {
        print_final_trace(client);
    }
    

    for(i=0;i<num_of_players;i++){
        close_players(players[i].sock);
    }
    
    for(i=0;i<num_of_players;i++){
        shutdown(players[i].sock,SHUT_RDWR);
        close(players[i].sock);
    }

    shutdown(socket_id,SHUT_RDWR);
    close(socket_id);
    free(player_sockets);
    free(players);

    sleep(10);

    return 0;
}
