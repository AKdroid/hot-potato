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

void write_to_socket(int s, char* payload);

void allocate_id(int client_socket, int id, char* hostname);

void register_client(int master,int *id);
#endif /* COMM_H */
