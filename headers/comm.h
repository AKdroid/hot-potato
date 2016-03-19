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

#endif /* COMM_H */
