#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void error(const char *msg,int type);
int read_line_fd(int fd, char *buffer, int max_len);