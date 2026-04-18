#include "user.h"

pthread_mutex_t user_lock = PTHREAD_MUTEX_INITIALIZER;

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

int make_user(char *username,char *password){
    pthread_mutex_lock(&user_lock);
    int fd=open("users",O_RDWR);
    if(fd<0){
        error("Error opening users file",1);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    if(strlen(username)>50||strlen(password)>50||strlen(username)==0||strlen(password)==0){
        error("Username and password must be less than 50 characters and greater than 0 characters",1);
        close(fd);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    //check if username exists
    int exists=0;
    lseek(fd,0,SEEK_SET);
    char buffer[100];
    while(read_line_fd(fd, buffer, sizeof(buffer)) > 0){
        char *token = strtok(buffer, ":");

        if(token && strcmp(token, username) == 0){
            exists = 1;
            break;
        }
    }
    if(exists){
        error("Username already exists",1);
        close(fd);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    //write to file
    lseek(fd,0,SEEK_END);
    bzero(buffer,sizeof(buffer));
    sprintf(buffer,"%s:%s\n",username,password);
    if(write(fd,buffer,strlen(buffer))<0){
        error("Error writing to users file",1);
        close(fd);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    close(fd);
    pthread_mutex_unlock(&user_lock);
    return 0;
}

int user_check(char *username,char *password){
    int fd=open("users",O_RDONLY);
    if(fd<0){
        error("Error opening users file",1);
        return 1;
    }
    int valid=0;
    lseek(fd,0,SEEK_SET);
    char buffer[100];
    while(read_line_fd(fd, buffer, sizeof(buffer)) > 0){
        char *token = strtok(buffer, ":");

        if(token && strcmp(token, username) == 0){
            valid = 1;
            break;
        }
    }
    if(!valid){
        error("Invalid username",1);
        close(fd);
        return 1;
    }
    close(fd);
    return 0;
}