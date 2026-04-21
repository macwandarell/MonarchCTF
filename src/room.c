#include "room.h"

pthread_mutex_t room_lock = PTHREAD_MUTEX_INITIALIZER;

int make_room(char *code,char *owner){
    struct Room room;
    memset(&room,0,sizeof(room));
    strncpy(room.code,code,10);
    room.code[10]='\0';
    room.no_of_users=0;
    strncpy(room.room_owner,owner,50);
    for(int i=0;i<100;i++){
        room.solved_problems[i][0]='\0';
    }
    pthread_mutex_lock(&room_lock);
    int fd=open("data/rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    //check first if owner is already in a room
    lseek(fd,0,SEEK_SET);
    struct Room temp;
    memset(&temp,0,sizeof(temp));
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
    int fd=open("data/rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    struct Room room;
    int exists=0;
    lseek(fd,0,SEEK_SET);
    //first check wheter user already exists in any of the rooms other than the current one
    while(read(fd,&room,sizeof(room))>0){
        for(int i=0;i<room.no_of_users;i++){
            if(strncmp(room.users[i],username,50)==0&&strncmp(room.code,code,10)!=0){
                error("User already in a room",1);
                close(fd);
                pthread_mutex_unlock(&room_lock);
                return 1;
            }
            else if(strncmp(room.users[i],username,50)==0&&strncmp(room.code,code,10)==0){
                close(fd);
                pthread_mutex_unlock(&room_lock);
                return 0;
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
    int fd=open("data/rooms",O_RDONLY);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    lseek(fd,0,SEEK_SET);
    struct Room room;
    int offset=0;
    while(read(fd,&room,sizeof(struct Room))>0){
        offset+=sprintf(buffer+offset,"Room code: %.10s\n",room.code);
        offset+=sprintf(buffer+offset,"Room owner: %.49s\n",room.room_owner);
        offset+=sprintf(buffer+offset,"Number of users: %d\n",room.no_of_users);
        offset+=sprintf(buffer+offset,"Users in room:\n");
        for(int i=0;i<room.no_of_users;i++){
            offset+=sprintf(buffer+offset,"%.49s\n",room.users[i]);
        }
        offset+=sprintf(buffer+offset,"Solved problems:\n");
        for(int i=0;i<100;i++){
            if(room.solved_problems[i][0]!='\0'){
                offset+=sprintf(buffer+offset,"%.49s\n",room.solved_problems[i]);
            }
        }
        offset+=sprintf(buffer+offset,"\n");
    }
    buffer[offset]='\0';
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return 0;

}

int remove_room(char *code){
    pthread_mutex_lock(&room_lock);
    int fd=open("data/rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    int exists=0;
    lseek(fd,0,SEEK_SET);
    struct Room room;
    while(read(fd,&room,sizeof(room))>0){
        if(memcmp(room.code,code,10)==0){
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
    int temp_fd=open("data/temp_rooms",O_RDWR|O_CREAT|O_TRUNC,0666);
    if(temp_fd<0){
        pthread_mutex_unlock(&room_lock);
        error("Error opening temp room file",1);
        return 1;
    }
    lseek(fd,0,SEEK_SET);
    while(read(fd,&room,sizeof(room))>0){
        if(memcmp(room.code,code,10)==0){
            continue;}
        write(temp_fd,&room,sizeof(room));
    }
    close(fd);
    //remove rooms file and rename temp file to rooms
    if(remove("data/rooms")){
        error("Error removing rooms file",1);
        close(temp_fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    if(rename("data/temp_rooms","data/rooms")){
        error("Error renaming temp rooms file",1);
        close(temp_fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    close(temp_fd);
    pthread_mutex_unlock(&room_lock);
    return 0;
}


int update_solved(char *problem,char *code){
    pthread_mutex_lock(&room_lock);
    int fd=open("data/rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    struct Room room;
    int found=0;
    off_t room_offset;
    lseek(fd,0,SEEK_SET);
    while(read(fd,&room,sizeof(room))>0){
        if(memcmp(room.code,code,10)==0){
            found=1;
            room_offset=lseek(fd,0,SEEK_CUR)-sizeof(room);
            break;
        }
    }
    if(!found){
        error("Room code does not exist",1);
        close(fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    for(int i=0;i<100;i++){
    	if(strncmp(room.solved_problems[i],problem,50)==0){
    	close(fd);
    	error("Room is already solved",1);
    	pthread_mutex_unlock(&room_lock);
    	return 1;}}
    int empty_slot=-1;
    for(int i=0;i<100;i++){
        if(room.solved_problems[i][0]=='\0'){
            empty_slot=i;
            break;
        }
    }   
    if(empty_slot==-1){
        error("Maximum solved problems reached",1);
        close(fd);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    strncpy(room.solved_problems[empty_slot],problem,50);
    room.solved_problems[empty_slot][49]='\0';
    lseek(fd,room_offset,SEEK_SET);
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

int print_points(char* buffer, size_t buffer_size,char *code){
    pthread_mutex_lock(&room_lock);
    int fd=open("data/rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return -1;
    }
    struct Room room;
    lseek(fd,0,SEEK_SET);
    int offset=0;
    while(read(fd,&room,sizeof(struct Room))>0){
        if(memcmp(room.code,code,10)==0){
            offset += snprintf(buffer + offset, buffer_size - (size_t)offset, "Solved problems:\n");
            for(int i=0;i<100;i++){
                if(room.solved_problems[i][0]!='\0'){
                    offset += snprintf(buffer + offset, buffer_size - (size_t)offset, "%.49s\n", room.solved_problems[i]);
                }
                if ((size_t)offset >= buffer_size) {
                    offset = (int)buffer_size;
                    break;
                }
            }
            if ((size_t)offset < buffer_size) {
                offset += snprintf(buffer + offset, buffer_size - (size_t)offset, "\n");
            }
            break;
        }
    }
    if (buffer_size > 0) {
        if (offset < 0) offset = 0;
        if ((size_t)offset >= buffer_size) offset = (int)buffer_size - 1;
        buffer[offset]='\0';
    }
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return offset;


}

int remove_problem_room(char *problem){
    pthread_mutex_lock(&room_lock);
    int fd=open("data/rooms",O_RDWR);
    if(fd<0){
        error("Error opening rooms file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    struct Room room;
    off_t room_offset;
    lseek(fd,0,SEEK_SET);
    
    while(read(fd,&room,sizeof(room))>0){
        room_offset=lseek(fd,0,SEEK_CUR)-sizeof(room);
        for(int i=0;i<100;i++){
            if(strncmp(room.solved_problems[i],problem,50)==0){
                room.solved_problems[i][0]='\0';
                lseek(fd,room_offset,SEEK_SET);
                if(write(fd,&room,sizeof(room))<0){
                error("Error writing to rooms file",1);
                close(fd);
                pthread_mutex_unlock(&room_lock);
                return 1;
            }
            lseek(fd,room_offset+sizeof(room),SEEK_SET);
                break;
            }
        }
        }
    
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return 0;
}
