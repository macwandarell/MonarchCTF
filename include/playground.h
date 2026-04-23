#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "custom_error.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/sha.h>
#include <pty.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include "room.h"
#include <pwd.h>
#include <grp.h>
#include <sys/resource.h>
#include <linux/limits.h>
#include <sys/mount.h>





#define playground_buffer 10000
#define max_rooms 5

extern char* active_rooms[5];
extern int active_users[5];
extern int current_active;
extern pthread_mutex_t playground_lock;
extern pthread_mutex_t playground_problem_lock;
extern pthread_mutex_t playground_user_lock;
extern pthread_mutex_t playground_active_lock;

int run_playground(int newsockfd,char *code);
void new_room_setup(char *code);
int submit_handler(char *cmd_buffer,int newsockfd);
int points_handler(int newsockfd,char *code);
int add_new_problem(char *problem,char *solution);
int solutions_file_opener();
int remove_problem(char *problem);
int add_new_command(char *name);

#endif
