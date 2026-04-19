#include "room.h"

pthread_mutex_t room_lock = PTHREAD_MUTEX_INITIALIZER;

int make_room(char *code,char *owner){
    struct Room room;
    memset(&room,0,sizeof(room));
    strncpy(room.code,code,10);
    room.code[10]='\0';
    room.no_of_users=0;
    strncpy(room.room_owner,owner,50);
    pthread_mutex_lock(&room_lock);
    int fd=open("rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    //check first if owner is already in a room
    lseek(fd,0,SEEK_SET);
    struct Room temp;
    //first check wheter user already exists in any of the rooms
    while(read(fd,&temp,sizeof(temp))>0){
        for(int i=0;i<temp.no_of_users;i++){
            if(strncmp(temp.users[i],owner,50)==0){
                error("User already in a room",1);
                close(fd);
                pthread_mutex_unlock(&room_lock);
                return 1;
            }
        }
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
    pthread_mutex_lock(&room_lock);
    int fd=open("rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    struct Room room;
    int exists=0;
    lseek(fd,0,SEEK_SET);
    //first check wheter user already exists in any of the rooms
    while(read(fd,&room,sizeof(room))>0){
        for(int i=0;i<room.no_of_users;i++){
            if(strncmp(room.users[i],username,50)==0){
                error("User already in a room",1);
                close(fd);
                pthread_mutex_unlock(&room_lock);
                return 1;
            }
        }
    }
    lseek(fd,0,SEEK_SET);
    while(read(fd,&room,sizeof(room))>0){
        if(strncmp(room.code,code,10)==0){
            exists=1;
            break;
        }
    }
    if(!exists){
        error("Room code does not exist",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    if(room.no_of_users>=5){
        error("Room is full",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
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


int print_all_rooms(char *buffer){
    pthread_mutex_lock(&room_lock);
    int fd=open("rooms",O_RDONLY);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    lseek(fd,0,SEEK_SET);
    struct Room room;
    int offset=0;
    while(read(fd,&room,sizeof(struct Room))>0){
        offset+=sprintf(buffer+offset,"Room code: %s\n",room.code);
        offset+=sprintf(buffer+offset,"Room owner: %s\n",room.room_owner);
        offset+=sprintf(buffer+offset,"Number of users: %d\n",room.no_of_users);
        offset+=sprintf(buffer+offset,"Users in room:\n");
        for(int i=0;i<room.no_of_users;i++){
            offset+=sprintf(buffer+offset,"%s\n",room.users[i]);
        }
        offset+=sprintf(buffer+offset,"\n");
    }
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return 0;

}

int remove_room(char *code){
    pthread_mutex_lock(&room_lock);
    int fd=open("rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    int exists=0;
    lseek(fd,0,SEEK_SET);
    struct Room room;
    while(read(fd,&room,sizeof(room))>0){
        if(strncmp(room.code,code,10)==0){
            exists=1;
             break;
        }
    }
    if(!exists){
        error("Room code does not exist",1);
        close(fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    int temp_fd=open("temp_rooms",O_RDWR|O_CREAT|O_TRUNC,0666);
    if(temp_fd<0){
        pthread_mutex_unlock(&room_lock);
        error("Error opening temp room file",1);
        return 1;
    }
    lseek(fd,0,SEEK_SET);
    while(read(fd,&room,sizeof(room))>0){
        if(strncmp(room.code,code,10)==0){
            continue;}
        write(temp_fd,&room,sizeof(room));
    }
    close(fd);
    //remove rooms file and rename temp file to rooms
    if(remove("rooms")){
        error("Error removing rooms file",1);
        close(temp_fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    if(rename("temp_rooms","rooms")){
        error("Error renaming temp rooms file",1);
        close(temp_fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    close(temp_fd);
    pthread_mutex_unlock(&room_lock);
    return 0;
}