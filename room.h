#ifndef ROOM_H
#define ROOM_H

#include <stdio.h>
#include "custom_error.h"
#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>


extern pthread_mutex_t room_lock;


struct Room{
    char code[10];
    char users[5][50];
    int no_of_users;
    char room_owner[50];
};

int make_room(char *code,char *owner);
int user_join_room(char *code,char *username);

#endif