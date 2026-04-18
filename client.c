#include "client.h"

void sha256_cl(const char *input, char output[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
}

void client_run(){
    int sockfd,portno,n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char password[50];

    char buffer[255];
    printf("Enter servername: ");
    scanf("%s",buffer);
    server=gethostbyname(buffer);
    if(server==NULL){
        error("Server is off or ip is incorrect",0);
    }
    printf("Enter portno: ");
    scanf("%s",buffer);
    portno=atoi(buffer);
    getchar();
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        error("Error opening socket",0);
    }
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    bcopy((char*)server->h_addr, (char*) &server_addr.sin_addr.s_addr,server->h_length);
    server_addr.sin_port=htons(portno);
    if(connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        error("Connect function failed",0);
    }
    while(1){
        bzero(buffer,sizeof(buffer));
        n=read(sockfd,buffer,sizeof(buffer));
        if(n<0){
            error("Read failed",1);
            continue;
        }
        buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
        printf("%s",buffer);
        if(!check_exit(buffer,password)){printf("\nExiting the application because of server command\n");break;}
        fflush(stdout);
        int l=strcmp("Please enter password(server67): ",buffer);
        bzero(buffer,sizeof(buffer));
        fgets(buffer,sizeof(buffer),stdin);
        n=write(sockfd,buffer,strlen(buffer));
        if(n<0){
            error("Write failed",1);
            printf("Try restarting....\n");
            continue;
        }
        if(l==0){
            strcpy(password,buffer);
        }
    }
    close(sockfd);
    return;

}

int check_exit(char *buffer,char *password){
    char command[10];
    int random_number;
    char received_hash[65];
    sscanf(buffer, "%[^|]|%d|%s", command, &random_number, received_hash);
    char data[256];
    sprintf(data, "EXIT%d%s", random_number, password);
    char computed_hash[65];
    sha256_cl(data,computed_hash);
    if(strcmp(command,"EXIT")==0 && strcmp(received_hash,computed_hash)==0){
        return 0;}
    return 1;}

