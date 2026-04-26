#include "user.h"

pthread_mutex_t user_lock = PTHREAD_MUTEX_INITIALIZER;

int make_user(char *username,char *password){
    if(strcmp(username,"admin")==0){
        error("Username cannot be admin",1);
        return 2;
    }
    pthread_mutex_lock(&user_lock);
    int fd=open("data/users",O_RDWR);
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
    sprintf(buffer,"%s:%s:0\n",username,password);
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
    pthread_mutex_lock(&user_lock);
    if(strcmp(username,"admin")==0&&strcmp(password,"admin")==0){
        int ok=admin_login();
        if(ok){
            error("Admin already logged in",1);
            pthread_mutex_unlock(&user_lock);
            return 1;
        }
        pthread_mutex_unlock(&user_lock);
        return 2;
    }       
    int fd=open("data/users",O_RDWR);
    if(fd<0){
        error("Error opening users file",1);
        return 1;
    }
    int valid=0;
    lseek(fd,0,SEEK_SET);
    char buffer[100];
    while(read_line_fd(fd, buffer, sizeof(buffer)) > 0){
        char *token1 = strtok(buffer, ":");
        char *token2 = strtok(NULL, ":");
        char *token3 = strtok(NULL, ":");

        if(token1 && strcmp(token1, username) == 0&&strcmp(token2,password)==0&& strcmp(token3,"0")==0){
            off_t offset = lseek(fd, 0, SEEK_CUR) - strlen(token3) - 1;
            lseek(fd, offset, SEEK_SET);
            if(write(fd,"1",1)<0){
                error("Error writing to users file",1);
                close(fd);
                pthread_mutex_unlock(&user_lock);
                return 1;}
            valid = 1;
            break;
        }
    }
    if(!valid){
        error("Invalid username or user is logged in",1);
        close(fd);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    close(fd);
    pthread_mutex_unlock(&user_lock);
    return 0;
}

int user_logout(char *username){
    if(strcmp(username,"admin")==0){
        admin_logout();
        return 0;
    }
    pthread_mutex_lock(&user_lock);
    int fd=open("data/users",O_RDWR);
    if(fd<0){
        error("Error opening users file",1);
        return 1;
    }
    int valid=0;
    lseek(fd,0,SEEK_SET);
    char buffer[100];
    while(read_line_fd(fd,buffer,sizeof(buffer))>0){
        char *token1=strtok(buffer,":");
        char *token2=strtok(NULL,":");
        char *token3=strtok(NULL,":");
        if(token1&&strcmp(token1,username)==0&&strcmp(token3,"1")==0){
            off_t offset=lseek(fd,0,SEEK_CUR)-strlen(token3)-1;
            lseek(fd,offset,SEEK_SET);
            if(write(fd,"0",1)<0){
                error("Error writing to users file",1);
                close(fd);
                pthread_mutex_unlock(&user_lock);
                return 1;}
            valid=1;
            break;
        }
    }
    if(!valid){
        error("User not found or not logged in",1);
    }
    close(fd);
    pthread_mutex_unlock(&user_lock);
    return 0;
}

int print_all_users(char *buffer){
    pthread_mutex_lock(&user_lock);
    int fd=open("data/users",O_RDONLY);
    if(fd<0){
        error("Error opening users file",1);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    lseek(fd,0,SEEK_SET);
    char temp[100];
    while(read_line_fd(fd,temp,sizeof(temp))>0){
        char *token1=strtok(temp,":");
        char *token2=strtok(NULL,":");
        char *token3=strtok(NULL,":");
        if(token1&&token2&&token3){
            strcat(buffer,token1);
            strcat(buffer," - ");
            strcat(buffer,strcmp(token3,"1")==0?"Logged in":"Logged out");
            strcat(buffer,"\n");
        }
    }
    close(fd);
    pthread_mutex_unlock(&user_lock);
    return 0;
}

int user_remove(char *username){
    pthread_mutex_lock(&user_lock);
    int fd=open("data/users",O_RDWR);
    if(fd<0){
        error("Error opening users file",1);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    int found=0;
    lseek(fd,0,SEEK_SET);
    char buffer[100];
    while(read_line_fd(fd,buffer,sizeof(buffer))>0){
        char *token= strtok(buffer,":");
        if(token&&strcmp(token,username)==0){
            found=1;
            break;
        }
    }
    if(!found){
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    //make temp file and move all other users to it
    int temp_fd=open("data/temp_users",O_RDWR|O_CREAT|O_TRUNC,0666);
    if(temp_fd<0){
        pthread_mutex_unlock(&user_lock);
        error("Error opening temp file",1);
        return 1;
    }
    lseek(fd,0,SEEK_SET);
    while(read_line_fd(fd,buffer,sizeof(buffer))>0){
        char temp[100];
        strcpy(temp,buffer);
        char *token=strtok(temp,":");
        if(token&&strcmp(token,username)==0){
            continue;
        }
        write(temp_fd,buffer,strlen(buffer));
        write(temp_fd,"\n",1);
    }
    close(fd);
    //remove users file and rename temp file to users
    if(remove("data/users")){
        error("Error removing users file",1);
        close(temp_fd);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    if(rename("data/temp_users","data/users")){
        error("Error renaming temp file",1);
        close(temp_fd);
        pthread_mutex_unlock(&user_lock);
        return 1;
    }
    close(temp_fd);
    pthread_mutex_unlock(&user_lock);
    return 0;
}
