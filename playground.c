#include "playground.h"

char* active_rooms[5]={NULL};
int current_acitve=0;


void new_room_setup(char *code){
    char path[256];
    snprintf(path,sizeof(path),"/home/ctf/%s",code);
    mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    char cmd[512];
    snprintf(cmd,sizeof(cmd),"cp -r /home/ctf/problems/* %s/",path);
    system(cmd);
}

int run_playground(int newsockfd,char *code){
    int exists=0;
    for(int i=0;i<max_rooms;i++){
        if(active_rooms[i]!=NULL && strncmp(code,active_rooms[i],10)==0){
            exists=1;
            break;
        }
    }
    if(!exists){
        if(current_acitve>=5){
            error("Active rooms limit reached",1);
            return 1;
        }
        active_rooms[current_acitve]=strdup(code);current_acitve++;
        new_room_setup(code);
    }
    int main_fd;
    pid_t pid;
    pid=forkpty(&main_fd,NULL,NULL,NULL);
    if(pid<0){
        error("forkpty error",1);
        return 1;
    }
    if(pid==0){
        execl("/bin/bash","bash",NULL);
        error("execl error",1);
        return 1;
    }
    char buffer[playground_buffer];

    while(1){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(newsockfd,&fds);
        FD_SET(main_fd,&fds);
        int maxfd=(newsockfd>main_fd?newsockfd:main_fd)+1;
        if(select(maxfd,&fds,NULL,NULL,NULL)<0){
            error("error in select in playground",1);
            return 1;
        }
        if(FD_ISSET(main_fd,&fds)){
            int n=read(main_fd,buffer,sizeof(buffer));
            if(n<=0)break;
            write(newsockfd,buffer,n);
        }
        if(FD_ISSET(newsockfd,&fds)){
            int n=read(newsockfd,buffer,sizeof(buffer));
            if(n<=0) break;
            write(main_fd,buffer,n);
        }
    }
    close(main_fd);
    wait(NULL);
    return 0;

}
