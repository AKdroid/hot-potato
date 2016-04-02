CC=gcc
CFLAGS= -Wall -Wimplicit
INCLUDE=-I headers
SRC=src
TARGET1=player
TARGET2=master

all: player master

player: src/player.c src/comm.c 
	$(CC) $(CFLAGS) $(SRC)/comm.c $(SRC)/player.c -o $(TARGET1) $(INCLUDE)

master: src/master.c src/comm.c 
	$(CC) $(CFLAGS) $(SRC)/comm.c $(SRC)/master.c -o $(TARGET2) $(INCLUDE)




