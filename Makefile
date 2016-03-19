CC=gcc
CFLAGS=-w -Wall
INCLUDE=-I headers
SRC=src
TARGET1=player
TARGET2=master

all: player master

player:
	$(CC) $(CFLAGS) $(SRC)/comm.c $(SRC)/player.c -o $(TARGET1) $(INCLUDE)

master:
	$(CC) $(CFLAGS) $(SRC)/comm.c $(SRC)/master.c -o $(TARGET2) $(INCLUDE)




