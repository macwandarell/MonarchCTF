#include "room.h"

pthread_mutex_t room_lock = PTHREAD_MUTEX_INITIALIZER;

int make_room(char *code,char *owner){
    struct Room room;
    strncpy(room.code,code,10);
    room.no_of_users=0;
    strncpy(room.room_owner,owner,50);
    pthread_mutex_lock(&room_lock);
    int fd=open("rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    lseek(fd,0,SEEK_END);
    if(write(fd,&room,sizeof(room))<0){
        error("Error writing to rooms file",1);
        close(fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return 0;
}


int user_join_room(char *code,char *username){
    int fd=open("rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        return 1;
    }
    struct Room room;
    int exists=0;
    lseek(fd,0,SEEK_SET);
    while(read(fd,&room,sizeof(room))>0){
        if(strncmp(room.code,code,10)==0){
            exists=1;
            break;
        }
    }
    if(!exists){
        error("Room code does not exist",1);
        return 1;
    }
    if(room.no_of_users>=5){
        error("Room is full",1);
        return 1;
    }
    pthread_mutex_lock(&room_lock);
    //add user
    strncpy(room.users[room.no_of_users],username,50);
    room.no_of_users++;
    //update 
    lseek(fd,-sizeof(room),SEEK_CUR);
    if(write(fd,&room,sizeof(room))<0){
        error("Error writing to rooms file",1);
        close(fd);       
        pthread_mutex_unlock(&room_lock);
        return 1;}
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return 0;
}