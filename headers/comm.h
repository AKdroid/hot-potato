#ifndef COMM_H
#define COMM_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/fcntl.h>
#include<sys/time.h>
#include <sys/types.h>



struct sockaddr_in* initialize_master(int portnum, int* s);

struct sockaddr_in* connect_to_master(char* hostname, int portnum, int* s);

int parse_command(char* buf, char* cmd, int *size , char** body);

void read_from_socket(int s, char** rcvd, int* rcvdsize);

int write_to_socket(int s, char* payload);

void allocate_id(int client_socket, int id, int lid, int rid, char* hostname);

void register_client(int master,int *id, int* lid, int *rid);

void setup_left(int s, int* listening_port);

void setup_right(int s, char* hostname, int portnum);

int test_connection(int s, int flag);

void connect_to_neighbor(int master, struct sockaddr_in** left, struct sockaddr_in** right, int* left_s, int* right_s);

void initiate_game(int player_sock, int hops);

int select_readable_socket(int * sockets, int length, int timeout_sec);

void close_players(int sock);

void print_final_trace(int sock);

void get_potato(int sock, int* hops, char ** content, char **mainbody);

void pass_potato(int sock, int id, int hops, int comma, char *body);
#endif /* COMM_H */
