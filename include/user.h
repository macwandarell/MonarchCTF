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
#include "admin.h"


extern pthread_mutex_t user_lock;


int make_user(char *username,char *password);
int user_check(char *username,char *password);
int user_logout(char *username);
int print_all_users(char *buffer);
int user_remove(char *username);
#endif