#ifndef ADMIN_H
#define ADMIN_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "custom_error.h"
#include "room.h"
#include "user.h"
#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/sha.h>

#define max_admin_buffer 10000
extern int admin_logged_in;


int admin_login();
void admin_logout();
int admin_handler(int newsockfd);
#endif