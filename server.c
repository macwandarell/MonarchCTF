#include "server.h"


void *handle_client(void *arg){
    //have to add client handling logic here
    //firstly, server has to give a choicde to client to create a room or join an existing room
    //since its a team based ctf, what the server will first do is it will generate a room code
    //it will share that room code to the client and then other client can join the same room using that code
    int newsockfd=*(int*) arg;
    char buffer[256];
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"Welcome to the MonarchCTF\nPlease enter room code to join or type 'create' if you want to make your own room:");
    if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket",1);
        close(newsockfd);
        return NULL;
    }
    bzero(buffer,sizeof(buffer));
    if(read(newsockfd,buffer,sizeof(buffer))<0){
        error("Error reading from socket",1);
        close(newsockfd);
        return NULL;
    }
    if(strncmp(buffer,"create",6)==0){
        if(create_room(newsockfd)){
            error("Error creating room",1);
            close(newsockfd);
            return NULL;
        }
    }
    else{
        if(join_room(newsockfd,buffer)){
            error("Error joining room",1);
            close(newsockfd);
            return NULL;
        }
    }
    close(newsockfd);
    free(arg);
    return NULL;
}

//start server and listen for new connection
void start_server(int portno){
    int room_file=open("room_codes",O_RDWR|O_CREAT,0666);
    if(room_file<0){
        error("Error opening room codes file",0);
    }
    close(room_file);
    int sockfd,*newsockfd,n;
    struct sockaddr_in server_addr,client_addr;
    pthread_t tid;
    socklen_t client_len;
    sockfd=socket(AF_INET,SOCK_STREAM,0);

    if(sockfd<0){
        error("Error opening socket.",0);
    }

    bzero((char *)&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(portno);

    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        error("Error on bind",0);
    }


    listen(sockfd,max_clients);

    while(1){
        //using pthreads for multiple clients
        client_len=sizeof(client_addr);
        newsockfd=malloc(sizeof(int));
        *newsockfd=accept(sockfd,(struct sockaddr *)&client_addr,&client_len);
        if(*newsockfd<0){
            error("Accept failed",1);
            continue;
        }
        pthread_create(&tid,NULL,handle_client,newsockfd);
        pthread_detach(tid);
    }
    close(sockfd);
    return;
}


int create_room(int newsockfd){
    char code[10];
    if(generate_room_code(code)){
        error("Error generating room code",1);
        return 1;
    }
    char *message="Room has been created, room code is: ";
    strcat(message,code);
    if((write(newsockfd,message,strlen(message)))<0){
        error("Error writing room code",1);
        return 1;
    }
    return 0;
}

int generate_room_code(char *code){
    //generate a code and check if it already exists
    //here we need to use locking as well because 2 clients can end up having the same code
    pthread_mutex_lock(&room_lock);

    int fd=open("room_codes",O_RDWR);
    if(fd<0){
        error("Error opening room codes file",1);
        pthread_mutex_unlock(&room_lock);
        return 1;
    }
    int exists;
    char room_code[10];
    do{
        exists=0;
        //generate a 10 digit alphanumeric code
        for(int i=0;i<10;i++){
            int r=rand()%36;
            if(r<10){
                room_code[i]='0'+r;}
            else{
                code[i]='A'+(r-10);
            }
            }
        //check if code already exists
        lseek(fd,0,SEEK_SET);
        char buffer[10];
        while(read(fd,buffer,sizeof(buffer))>0){
            if(strncmp(buffer,room_code,10)==0){
                exists=1;
                break;
            }
        }
        }while(exists);
    
    //write code to the file
    lseek(fd,0,SEEK_END);
    write(fd,room_code,10);
    strcpy(code,room_code);
    close(fd);
    pthread_mutex_unlock(&room_lock);
    return 0;
}


int join_room(int newsockfd,char *buffer){
    int fd=open("room_codes",O_RDONLY);
    if(fd<0){
        error("Error opening room codes file",1);
        return 1;
    }
    char code[10];
    strncpy(code,buffer,10);
    lseek(fd,0,SEEK_SET);
    int exists=0;
    while(read(fd,buffer,sizeof(buffer))>0){
        if(strncmp(buffer,code,10)==0){
            exists=1;
            break;
        }
    }
    close(fd);
    if(!exists){
        char *message="Room code not found. Please try again.";
        if(write(newsockfd,message,strlen(message))<0){
            error("Error writing to socket",1);
            return 1;
    }
    return 1;}
    //define what a room exactly is and then join
    //have to add more logic here
    return 0;
}

    



