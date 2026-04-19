#include "admin.h"

int admin_logged_in=0;

int admin_login(){
    if(admin_logged_in){
        return 1;
    }
    admin_logged_in=1;
    return 0;
}

void admin_logout(){
    admin_logged_in=0;
}


int admin_handler(int newsockfd){
    char buffer[max_admin_buffer];
    int n;
    while(1){
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"Welcome admin\n Options available:\n1. View all users\n2.View all rooms\n3.Remove a user\n4.Remove a room\n5.Add a problem to the playground\n6.Remove a problem from the playground\n7.Logout\n");
    if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket(admin options screen)",1);
        return 1;
    }
    n=read(newsockfd,buffer,sizeof(buffer));
    if(n<0){
        error("Error reading from socket(admin options screen)",1);
        return 1;
    }
    buffer[n]='\0';buffer[strcspn(buffer,"\r\n")]='\0';
    if(strcmp(buffer,"1")==0){
        bzero(buffer,sizeof(buffer));
        print_all_users(buffer);
        if(write(newsockfd,buffer,strlen(buffer))<0){
            error("Error writing to socket(admin view users screen)",1);
            return 1;
        }
        continue;
    }
    else if(strcmp(buffer,"2")==0){
        bzero(buffer,sizeof(buffer));
        print_all_rooms(buffer);
        if(write(newsockfd,buffer,strlen(buffer))<0){
            error("Error writing to socket(admin view rooms screen)",1);
            return 1;
        }
        continue;
    }
    else if(strcmp(buffer,"3")==0){
        bzero(buffer,sizeof(buffer));
        char username[50];
        strcpy(buffer,"Enter username to remove:");
        if(write(newsockfd,buffer,strlen(buffer))<0){
            error("Error writing to socket(admin remove user screen)",1);
            return 1;}
        n=read(newsockfd,buffer,sizeof(buffer));
        if(n<0){
            error("Error reading from socket(admin remove user screen)",1);
            return 1;}
        buffer[n]='\0';buffer[strcspn(buffer,"\r\n")]='\0';
        strncpy(username,buffer,49);
        username[49]='\0';

        if(user_remove(username)){
            if(write(newsockfd,"Error removing user. User doesnt exist or server issue\n",strlen("Error removing user. User doesnt exist or server issue\n"))<0){
                error("Error writing to socket(admin remove user failure screen)",1);
                return 1;
            }
            continue;

        }
        remove_active_user(username);
        if(write(newsockfd,"User removed\n",strlen("User removed\n"))<0){
            error("Error writing to socket(admin remove user success screen)",1);
            return 1;
        }
    }
    else if(strcmp(buffer,"4")==0){
        bzero(buffer,sizeof(buffer));
        char code[11];
        strcpy(buffer,"Enter room code to remove:");
        if(write(newsockfd,buffer,strlen(buffer))<0){
            error("Error writing to socket(admin remove room screen)",1);
            return 1;}
        n=read(newsockfd,buffer,sizeof(buffer));
        if(n<0){
            error("Error reading from socket(admin remove room screen)",1);
            return 1;}
        buffer[n]='\0';buffer[strcspn(buffer,"\r\n")]='\0';
        strncpy(code,buffer,10);
        code[10]='\0';
        if(remove_room(code)){
            if(write(newsockfd,"Error removing room.Room doesnt exist or server issue\n",strlen("Error removing room.Room doesnt exist or server issue\n"))<0){
                error("Error writing to socket(admin remove room failure screen)",1);
                return 1;
            }
            continue;
        }
        if(write(newsockfd,"Room removed\n",strlen("Room removed\n"))<0){
            error("Error writing to socket(admin remove room success screen)",1);
            return 1;
        }
        }
    else if(strcmp(buffer,"7")==0){
        admin_logout();
        break;
    }
    else{
        if(write(newsockfd,"Invalid option. Please try again.\n",strlen("Invalid option. Please try again.\n"))<0){
            error("Error writing to socket(admin invalid option screen)",1);
            return 1;
        }
        continue;
    }
    }
    return 0;
    }

