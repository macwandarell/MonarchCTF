#include "playground.h"

char* active_rooms[max_rooms]={NULL};
int active_users[max_rooms]={0};
pid_t room_pids[max_rooms][max_user_per_room]={{0}};
int current_active=0;
pthread_mutex_t playground_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t playground_problem_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t playground_user_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t playground_active_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t playground_room_pid_lock=PTHREAD_MUTEX_INITIALIZER;


void handle_termination(int sig){
	_exit(0);
}

void new_room_setup(char *code){
    pthread_mutex_lock(&playground_problem_lock);
    char path[256];
    snprintf(path,sizeof(path),"/home/ctf/%s",code);
    char prob_path[300];
    snprintf(prob_path,sizeof(prob_path),"%s/problems",path);
    mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(prob_path, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    char cmd[512];
    snprintf(cmd,sizeof(cmd),"cp -r /home/ctf/jail/* %s/",path);
    system(cmd);
      char real_share_path[350];
    snprintf(real_share_path,sizeof(real_share_path),"/home/ctf/%s_real",code);
    mkdir(real_share_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    bzero(cmd,sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"chown -R team_%s:team_%s /home/ctf/%s_real",code,code,code);
    system(cmd);
    /*bzero(cmd,sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"mountpoint -q /home/ctf/%s/dev/pts ||mount -t devpts devpts /home/ctf/%s/dev/pts",code,code);
    system(cmd);*/
    struct fuse_thread_args* fuse_args=malloc(sizeof(struct fuse_thread_args));
    snprintf(fuse_args->mountpoint,256,"%s",prob_path);
    snprintf(fuse_args->real_root,256,"/home/ctf/%s_real/problems",code);
    bzero(cmd,sizeof(cmd));
    char real_prob_path[400];
    snprintf(real_prob_path,sizeof(real_prob_path),"%s/problems",real_share_path);
    mkdir(real_prob_path, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    snprintf(cmd,sizeof(cmd),"cp -r /home/ctf/problems/* /home/ctf/%s_real/problems/",code);
    system(cmd);
    pid_t fuse_pid=fork();
    if(fuse_pid==0){
    	start_fuse(fuse_args);
    	exit(0);
    }
    pthread_mutex_unlock(&playground_problem_lock);
}

int add_new_problem(char *problem,char *solution){
    pthread_mutex_lock(&playground_problem_lock);
    //check if exist already
    char path[256];
    snprintf(path,sizeof(path),"/home/ctf/problems/%s",problem);
    if(access(path,F_OK)==0){
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    bzero(path,sizeof(path));
    snprintf(path,sizeof(path),"questions/%s",problem);
    char cmd[512];
    snprintf(cmd,sizeof(cmd),"cp -r %s /home/ctf/problems",path);
    system(cmd);
    int fd=solutions_file_opener();
    if(fd<0){
        error("Error opening solutions file",1);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    lseek(fd,0,SEEK_END);
    dprintf(fd,"%s %s\n",problem,solution);
    pthread_mutex_unlock(&playground_lock);
    pthread_mutex_unlock(&playground_problem_lock);
    return 0;
}
int remove_problem(char *problem){
    pthread_mutex_lock(&playground_problem_lock);
    char path[256];
    snprintf(path,sizeof(path),"/home/ctf/problems/%s",problem);
    char cmd[512];
    snprintf(cmd,sizeof(cmd),"rm -r %s",path);
    system(cmd);
    int fd=solutions_file_opener();
    if(fd<0){
        error("Error opening solutions file",1);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    int temp_fd=open("data/temp_solutions",O_WRONLY|O_CREAT|O_TRUNC,0666);
    if(temp_fd<0){
        error("Error opening temp solutions file",1);
        close(fd);
        pthread_mutex_unlock(&playground_lock);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    char buffer[512];
    while(read_line_fd(fd,buffer,sizeof(buffer))>0){
        char *prob=strtok(buffer," \n");
        char *ans=strtok(NULL,"\n");
        if(prob && ans){
            if(strcmp(prob,problem)!=0){
                dprintf(temp_fd,"%s %s\n",prob,ans);
            }
        }
    }
    close(fd);
    close(temp_fd);
    
    if(remove("data/solutions")){
        error("Error removing solutions file",1);
        pthread_mutex_unlock(&playground_lock);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    
    if(rename("data/temp_solutions","data/solutions")){
        error("Error renaming temp solutions file",1);
        pthread_mutex_unlock(&playground_lock);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    pthread_mutex_unlock(&playground_lock);
    pthread_mutex_unlock(&playground_problem_lock);
    
    if(remove_problem_room(problem)){
        error("Error in remove problems from room",1);
        return 1;
    }

    return 0;
}


int run_playground(int newsockfd,char *code){
    pthread_mutex_lock(&playground_active_lock);
    int exists=-1;
    for(int i=0;i<max_rooms;i++){
        if(active_rooms[i]!=NULL && strncmp(code,active_rooms[i],10)==0){
            active_users[i]+=1;
            exists=i;
            break;
        }
    }
    if(exists==-1){
        if(current_active>=max_clients){
            error("Active rooms limit reached",1);
            return 1;
        }
        active_rooms[current_active]=strdup(code);
        active_users[current_active]=1;
        exists=current_active;
        current_active++;
        new_room_setup(code);
    }
    pthread_mutex_unlock(&playground_active_lock);
    int main_fd;
    pid_t pid;
    pid=forkpty(&main_fd,NULL,NULL,NULL);
    if(pid<0){
        error("forkpty error",1);
        return 1;
    }
    if(pid==0){
    	setsid();
    	setpgid(0,0);
    	closefrom(3);
        pthread_mutex_lock(&playground_user_lock);
        char username[64];
        char cmd[512];
        snprintf(username,sizeof(username),"team_%s",code);
        struct passwd *pw=getpwnam(username);
        if(!pw){
        snprintf(cmd,sizeof(cmd),"useradd -M -d /home/ctf/%s -s /bin/bash %s",code,username);
        system(cmd);
        pw=getpwnam(username);
        if(!pw){
            error("getpwnam failed after adding user",1);
            pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }}
        bzero(cmd,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"chown -R %s:%s /home/ctf/%s",username,username,code);
        system(cmd);
        char root_path[256];
        snprintf(root_path,sizeof(root_path),"/home/ctf/%s",code);
        if(chdir(root_path)!=0){
            error("chdir failed",1);
       	    pthread_mutex_unlock(&playground_user_lock);
            return 1;
            
        }
        unshare(CLONE_NEWNS);
	mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL);
	char devpts_path[256];
	snprintf(devpts_path, sizeof(devpts_path), "/home/ctf/%s/dev/pts", code);
	mount("devpts", devpts_path, "devpts", 0, "newinstance,ptmxmode=0666,mode=0620");
        if(chroot(".")!=0){
            error("chroot failed",1);
            pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }
        if(chdir("/")!=0){
            error("chdir failed",1);
                    	pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }
        
        if(!pw){
            error("getpwnam failure",1);
                    	pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }
        if(setgroups(0,NULL)!=0){
            error("setgroups failure",1);
                    	pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }
        if(setgid(pw->pw_gid)!=0){
            error("dropping priveliges failed",1);
                    	pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }
        if(setuid(pw->pw_uid)!=0){
            error("dropping priveliges failed",1);
                    	pthread_mutex_unlock(&playground_user_lock);
            return 1;
        }
        struct rlimit rl;
        //memory limit
        rl.rlim_cur=256*1024*1024;
        rl.rlim_max=256*1024*1024;
        setrlimit(RLIMIT_AS,&rl);
	setenv("TERM","xterm-256color",1);
        pthread_mutex_unlock(&playground_user_lock);
        signal(SIGTERM,handle_termination);
        execl("/bin/bash","bash","-i",NULL);
        error("execl error",1);
        return 1;
    }
    pthread_mutex_lock(&playground_room_pid_lock);
    for(int j = 0; j <max_user_per_room; j++) {
        if(room_pids[exists][j]==0) {
            room_pids[exists][j]=pid;
            break;
        }
    }
    pthread_mutex_unlock(&playground_room_pid_lock);
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
            int total=0;
            while(total<n){
            int w=write(newsockfd,buffer+total,n-total);
            if(w<=0){
                error("error occured in writing in run_playground",1);
                return 1;
            }
            total+=w;}
        }
        if(FD_ISSET(newsockfd,&fds)){
            int n=read(newsockfd,buffer,sizeof(buffer));
            if(n<=0) break;
            int total=0;
            while(total<n){
            int w=write(main_fd,buffer+total,n-total);
            if(w<=0){
            error("write to pty failed",1);
            return 1;
        }
        total+=w;}
    }}
    pthread_mutex_lock(&playground_active_lock);
    if(active_users[exists]==1){
    close(main_fd);
    wait(NULL);
    printf("[DEBUG] Entered the most critical section\n");
    for(int i=0;i<max_rooms;i++){
        if(active_rooms[i]!=NULL && strncmp(code,active_rooms[i],10)==0){
            free(active_rooms[i]);
            for(int j=i;j<current_active-1;j++){
                active_rooms[j]=active_rooms[j+1];
            }
            active_rooms[current_active-1]=NULL;
            current_active--;
            break;
        }
    }
    char path[256];
    snprintf(path,sizeof(path),"/home/ctf/%s",code);
    char cmd[512];
    /*
    snprintf(cmd,sizeof(cmd),"umount -l /home/ctf/%s/dev/pts",code);
    system(cmd);*/
    snprintf(cmd,sizeof(cmd),"fusermount -uz %s/problems",path);
    system(cmd);
    bzero(cmd,sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"rm -r %s",path);
    system(cmd);
    bzero(cmd,sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"rm -r /home/ctf/%s_real",code);
    system(cmd);
	pthread_mutex_unlock(&playground_active_lock);
	active_users[exists]-=1;
	pthread_mutex_lock(&playground_room_pid_lock);
    for(int i=0;i<max_user_per_room;i++) {
    if(room_pids[exists][i]==pid) {
        room_pids[exists][i]=0;
        break;
    	}
	}
    pthread_mutex_unlock(&playground_room_pid_lock);
	return 0;}
    active_users[exists]-=1;
    pthread_mutex_unlock(&playground_active_lock);
    pthread_mutex_lock(&playground_room_pid_lock);
    for(int i=0;i<max_user_per_room;i++) {
    if(room_pids[exists][i]==pid) {
        room_pids[exists][i]=0;
        break;
    	}
	}
    pthread_mutex_unlock(&playground_room_pid_lock);
    close(main_fd);
    wait(NULL);
    return 0;

}



//the solutions logic i am cutting off for now
//i am doing simply the user can check the solutions and the points using room code
//to do something better,i actually have to make another script which simply can call these below functions when the user does submit or points
//the script in itself is correct, it just needs to be wrapped around something to make it work
//something like a helper script running in the shell which can call these backend functions



int solutions_file_opener(){
    pthread_mutex_lock(&playground_lock);
    int fd=open("data/solutions",O_RDWR);
        if(fd<0){
        error("Error opening solutions file",1);
        pthread_mutex_unlock(&playground_lock);
        return -1;
    }
    return fd;
}
int submit_handler(char *cmd_buffer,int newsockfd){
    int fd=solutions_file_opener();
    if(fd==-1){return 1;}
    char *code=strtok(cmd_buffer,":\n");
    char *prob=strtok(NULL,":\n");
    char *ans=strtok(NULL,":\n");

    if(!code||!prob||!ans||(strlen(code)!=10)){
        error("Invalid format",1);
        if(write(newsockfd,"Invalid format\n",sizeof("Invalid format\n"))<0){
            error("Write issue in submit handler",1);
            pthread_mutex_unlock(&playground_lock);
            return 1;
        }
        pthread_mutex_unlock(&playground_lock);
        return 1;
    }
    char buffer[512];
    int found=0;
    int correct=0;
    while(read_line_fd(fd,buffer,sizeof(buffer))>0){
        char *problem=strtok(buffer," \n");
        char *answer=strtok(NULL,"\n");
        if(problem&&answer){
            if(strcmp(problem,prob)==0){
                found=1;
                if(strcmp(answer,ans)==0){
                    correct=1;
                }
                break;
            }
        }
    }
    close(fd);
    pthread_mutex_unlock(&playground_lock); 
    if(correct){
        if(write(newsockfd,"Correct answer!\n",sizeof("Correct answer!\n"))<0){
            error("Error in submit write",1);
            return 1;
        }
        if(update_solved(prob,code)){
            error("Error updating score",1);
            if(write(newsockfd,"Error updating, resubmit answer\n",sizeof("Error updating, resubmit answer\n"))<0){
            error("Error in submit write",1);
            return 1;
        }
        }
    }
    else if (found){
        if(write(newsockfd,"Solution not correct\n",sizeof("Solution not correct\n"))<0){
            error("Error in submit write",1);
            return 1;
        }
    }
    else{
        if(write(newsockfd,"Problem not found\n",sizeof("Problem not found\n"))<0){
            error("Error in submit write",1);
            return 1;
        }
    }
    return 0;

}


int points_handler(int newsockfd,char *code){
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int n = print_points(buffer, sizeof(buffer), code);
    if(n<0){
        error("Error occured in room, no need to pause client",1);
        return 0;
    }
    if (n < 0) n = 0;
    if(write(newsockfd, buffer, (size_t)n) < 0){
        error("Error in writing points",1);
        return 1;
    }
    return 0;
}


int add_new_command(char *name){
    pthread_mutex_lock(&playground_problem_lock);
    char src[PATH_MAX];
    char real_path[PATH_MAX];
    char cmd[8192];
    
    snprintf(src,sizeof(src),"/bin/%s",name);

    if(access(src,X_OK)!=0){
        error("File doesnt exists",1);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    if(!realpath(src,real_path)){
        error("Error resolving path",1);
        pthread_mutex_unlock(&playground_problem_lock);
        return 1;
    }
    snprintf(cmd,sizeof(cmd),"cp -n '%s' /home/ctf/jail/bin/",real_path);
    system(cmd);

    snprintf(cmd,sizeof(cmd),"ldd '%s' | awk '{print $3}' | grep '/' | xargs -I{} cp -n {} /home/ctf/jail/lib/",real_path);
    system(cmd);
    system("cp -n /lib64/ld-linux-x86-64.so.2 /home/ctf/jail/lib64/");
    pthread_mutex_unlock(&playground_problem_lock);
    return 0;

}

void kick_users(char *code){
	pid_t local_pid_copy[max_user_per_room];
	int count=0;
	int room_index=-1;
	pthread_mutex_lock(&playground_room_pid_lock);
	for(int i=0;i<max_rooms;i++){
		if(active_rooms[i]&&strncmp(active_rooms[i],code,10)==0){
		room_index=i;
		for(int j=0;j<max_user_per_room;j++){
			if(room_pids[i][j]>0){
				local_pid_copy[count++]=room_pids[i][j];
	
			}}
			break;
		}}
	pthread_mutex_unlock(&playground_room_pid_lock);
	for(int i=0;i<count;i++){
		pid_t pid=local_pid_copy[i];
		printf("[DEBUG] kicking from room %s with pid %d\n",code,pid);
		if(kill(-pid,SIGTERM)!=0){printf("[DEBUG]sigterm failed\n");}
		sleep(1);
		kill(-pid, SIGKILL);
	}
	
}


