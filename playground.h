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




#define playground_buffer 10000
#define max_rooms 5

extern char* active_rooms[5];
extern int current_active;

int run_playground(int newsockfd,char *code);
void new_room_setup(char *code);


