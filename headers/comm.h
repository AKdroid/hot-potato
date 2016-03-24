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

void allocate_id(int client_socket, int id, char* hostname);

void register_client(int master,int *id);

void setup_left(int s, int* listening_port);

void setup_right(int s, char* hostname, int portnum);

int test_connection(int s, int flag);

void connect_to_neighbor(int master, struct sockaddr_in** left, struct sockaddr_in** right, int* left_s, int* right_s);
#endif /* COMM_H */
