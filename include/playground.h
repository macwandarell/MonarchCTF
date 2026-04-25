#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#define _GNU_SOURCE
#include <sched.h>
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
#include "fuser.h"
#include <signal.h>




#define playground_buffer 10000
#define max_rooms 5
#define max_user_per_room 5

extern char* active_rooms[max_rooms];
extern int active_users[max_rooms];
extern pid_t room_pids[max_rooms][max_user_per_room];
extern int current_active;
extern pthread_mutex_t playground_lock;
extern pthread_mutex_t playground_problem_lock;
extern pthread_mutex_t playground_user_lock;
extern pthread_mutex_t playground_active_lock;
extern pthread_mutex_t playground_room_pid_lock;

int run_playground(int newsockfd,char *code);
void new_room_setup(char *code);
int submit_handler(char *cmd_buffer,int newsockfd);
int points_handler(int newsockfd,char *code);
int add_new_problem(char *problem,char *solution);
int solutions_file_opener();
int remove_problem(char *problem);
int add_new_command(char *name);
void kick_users(char* code);
void handle_termination(int sig);

#endif
