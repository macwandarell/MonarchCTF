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

#define max_clients 5
#define buffer_size 300


void start_server();
void *handle_client(void *arg);
int create_room(int newsockfd,char *owner);
int join_room(int newsockfd,char *buffer,char *current_user);
int generate_room_code(char *code);
void sha256_sv(const char *input, char output[65]);
