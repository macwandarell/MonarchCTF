#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include "custom_error.h"
#include "room.h"
#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include "admin.h"
#include <signal.h>

#define max_clients 5
#define buffer_size 300
struct active_users{
    char username[50];
    int socket;
};

enum State{
    AUTH,
    ROOM,
    PLAY,
    LOGGED_EXIT,
    EXIT
};

extern struct active_users active_user_list[max_clients];
extern pthread_mutex_t server_lock;
extern int current_active_users;

void start_server();
void *handle_client(void *arg);
int create_room(int newsockfd,char *owner);
int join_room(int newsockfd,char *buffer,char *current_user);
int generate_room_code(char *code);
void sha256_sv(const char *input, char output[65]);
int remove_active_user(char *username);
void handle_signal(int sig);

#endif