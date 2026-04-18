#ifndef USER_H
#define USER_H
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


extern pthread_mutex_t user_lock;
int make_user(char *username,char *password);
int user_check(char *username,char *password);
int read_line_fd(int fd, char *buffer, int max_len);

#endif