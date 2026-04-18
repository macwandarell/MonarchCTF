#include <stdio.h>
#include "custom_error.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#define max_clients 5

pthread_mutex_t room_lock = PTHREAD_MUTEX_INITIALIZER;

void start_server(int portno);
void *handle_client(void *arg);
int create_room(int newsockfd);
int join_room(int newsockfd,char *buffer);
int generate_room_code(char *code);
