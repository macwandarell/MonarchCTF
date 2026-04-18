#include "server.h"

//SHA-256 function for exit code
void sha256_sv(const char *input, char output[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
}

void *handle_client(void *arg){
    //have to add client handling logic here
    //firstly, server has to give a choicde to client to create a room or join an existing room
    //since its a team based ctf, what the server will first do is it will generate a room code
    //it will share that room code to the client and then other client can join the same room using that code
    int newsockfd=*(int*) arg;
    int n;
    char buffer[buffer_size];
    char current_user[50];
    char current_user_password[50];
    int logged_in=0;
    while(1){
    bzero(buffer,sizeof(buffer));
    strcpy(buffer,"Welcome to MonarchCTF! Please enter 1 if you want to register or 2 if you want to login:\n");
    if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket(welcome screen)",1);
        break;
    }
    bzero(buffer,sizeof(buffer));
    n=read(newsockfd,buffer,sizeof(buffer));
    if(n<0){
        error("Error reading from socket(welcome screen)",1);
        break;
    }
    buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
    if(strcmp(buffer,"1")!=0 && strcmp(buffer,"2")!=0){
        error("Invalid option selected by client",1);
        continue;
    }
    else if(strcmp(buffer,"1")==0){
        char username[50],password[50];
        strcpy(buffer,"Please enter username:");
        if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket(registration screen)",1);
        break;
        }
        n=read(newsockfd,buffer,sizeof(buffer));
        if(n<0){
        error("Error reading from socket(registration screen)",1);
       break;
        }
        buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
        strncpy(username,buffer,50);
        strcpy(buffer,"Please enter password(server67): ");
        if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket(registration screen)",1);
        break;}
        n=read(newsockfd,buffer,sizeof(buffer));
        if(n<0){
        error("Error reading from socket(registration screen)",1);
        break;
        }
        buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
        strncpy(password,buffer,50);
        if(make_user(username,password)){
            error("Error creating user",1);
            bzero(buffer,sizeof(buffer));
            strcpy(buffer,"Username already exists or invalid attempt. Please try again.\n");
            if(write(newsockfd,buffer,strlen(buffer))<0){
                error("Error writing to socket(registration failure screen)",1);
                break;
            }
            continue;
        }
        bzero(buffer,sizeof(buffer));
        strncpy(buffer,"User created successfully. Please login to continue.\n",strlen("User created successfully. Please login to continue.\n"));
        if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket(login to continue screen)",1);
        break;}
        continue;
    }
    else if(strcmp(buffer,"2")==0){
            char username[50],password[50];
            strcpy(buffer,"Please enter username:");
            if(write(newsockfd,buffer,strlen(buffer))<0){
            error("Error writing to socket(login screen)",1);
            break;}
            n=read(newsockfd,buffer,sizeof(buffer));
            if(n<0){
            error("Error reading from socket(login screen)",1);
            break;}
            buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
            strncpy(username,buffer,50);
            strcpy(buffer,"Please enter password:");
            if(write(newsockfd,buffer,strlen(buffer))<0){
            error("Error writing to socket(login screen)",1);
            break;}
            n=read(newsockfd,buffer,sizeof(buffer));
            if(n<0){
            error("Error reading from socket(login screen)",1);
            break;}
            buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
            strncpy(password,buffer,50);
            int valid=user_check(username,password);
            if(valid==1){
                if(write(newsockfd,"Incorrect credentials.Please try again.",strlen("Incorrect credentials.Please try again."))<0){
                    error("Error writing to socket(incorrect credentials screen)",1);
                    break;
                }
                continue;
            }
            //correct credentials, set current user and move on
            logged_in=1;
            strncpy(current_user,username,50);
            strncpy(current_user_password,password,50);
    }
    if(logged_in){
    strcpy(buffer,"Logged in,Please enter room code to join or type 'create' if you want to make your own room:");
    if(write(newsockfd,buffer,strlen(buffer))<0){
        error("Error writing to socket",1);
        break;
    }
    bzero(buffer,sizeof(buffer));
    n=read(newsockfd,buffer,sizeof(buffer));
    if(n<0){
        error("Error reading from socket",1);
        break;
    }
    buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
    if(strncmp(buffer,"create",6)==0){
        if(create_room(newsockfd,current_user)){
            error("Error creating room",1);
            break;
        }
    }
    else{
        if(join_room(newsockfd,buffer,current_user)){
            error("Error joining room",1);
            break;
        }
    }
    //after joining or creating room, finally add the actual ctf logic, will make this soon i guess :)
        //to end the connection, what we will do is make a hash which is hash=sha256("EXIT"+random_number+user_password) and then send it to client
        int random_number=rand()%1000000;
    char exit_hash[65];
    char data[buffer_size];
    sprintf(data,"EXIT%d%s",random_number,current_user_password);
    sha256_sv(data,exit_hash);
    char message[321];
    sprintf(message,"EXIT|%d|%s",random_number,exit_hash);
    if(write(newsockfd,message,strlen(message))<0){
        error("Error writing exit code to socket",1);
        return NULL;
    }
    break;
}
}
    close(newsockfd);
    free(arg);
    return NULL;
}

//start server and listen for new connection
void start_server(){
    printf("Enter a port number to start the server: ");
    int portno;
    scanf("%d",&portno);
    int room_file=open("room_codes",O_RDWR|O_CREAT,0666);
    if(room_file<0){
        error("Error opening room codes file",0);
    }
    int user_file=open("users",O_RDWR|O_CREAT,0666);
    if(user_file<0){
        error("Error opening users file",0);
    }
    int room_details_file=open("rooms",O_RDWR|O_CREAT,0666);
    if(room_details_file<0){
        error("Error opening rooms file",0);
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


int create_room(int newsockfd,char *owner){
    char code[10];
    if(generate_room_code(code)){
        error("Error generating room code",1);
        return 1;
    }
    make_room(code,owner);
    char message[100];
    sprintf(message, "Room has been created, room code is: %s", code);
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
                room_code[i]='A'+(r-10);
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


int join_room(int newsockfd,char *buffer,char *current_user){
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
    if(user_join_room(code,current_user)){
        error("Error joining room",1);
        return 1;}
    char *message="Joined room successfully.";
    if(write(newsockfd,message,strlen(message))<0){
        error("Error writing to socket",1);
        return 1;
    }
    return 0;
}

    



