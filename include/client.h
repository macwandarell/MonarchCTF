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
#include <sys/select.h>
#include <termios.h>
#include <signal.h>



void client_run();
int check_exit(char *buffer,char *password);
void sha256_cl(const char *input, char output[65]);
void enable_raw();
void signal_handler(int sig);
