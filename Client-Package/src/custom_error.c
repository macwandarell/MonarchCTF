#include "custom_error.h"

void error(const char *msg,int type){
    if(type==0){
    perror(msg);
    exit(1);}
    else if (type==1){
        perror(msg);
    }
}

int read_line_fd(int fd, char *buffer, int max_len) {
    int i = 0;
    char c;

    while (i < max_len - 1) {
        int n = read(fd, &c, 1);
        if (n <= 0) break;
        if (c == '\n') break;
        buffer[i++] = c;
    }

    buffer[i] = '\0';
    return i;
}