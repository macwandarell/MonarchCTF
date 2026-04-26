#include "client.h"

struct termios orig, raw_mode;
int raw_mode_enabled=0;

void enable_raw(){
	if(raw_mode_enabled)return;
    	tcgetattr(0,&orig);
    	raw_mode=orig;
    	cfmakeraw(&raw_mode);
    	tcsetattr(0,TCSANOW,&raw_mode);
    	raw_mode_enabled=1;
 }
 
 void disable_raw(){
 if(raw_mode_enabled){
 tcsetattr(0,TCSANOW,&orig);
 raw_mode_enabled=0;}}
 
void signal_handler(int sig){
	disable_raw();
	exit(0);
}
void sha256_cl(const char *input, char output[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
}

void client_run(){
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
    int sockfd,portno,n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char password[51];
    bzero(password, sizeof(password));

    char buffer[255];
    printf("Enter servername: ");
    fgets(buffer,sizeof(buffer),stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    server=gethostbyname(buffer);
    if(server==NULL){
        error("Server is off or ip is incorrect",0);
    }
    printf("Enter portno: ");
    fgets(buffer,sizeof(buffer),stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    portno=atoi(buffer);
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
        int skip_stdin = 0;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd,&fds);
        FD_SET(0,&fds);
        int maxfd=(sockfd>0?sockfd: 0)+1;

        if(select(maxfd,&fds,NULL,NULL,NULL)<0){
            error("Select error",0);
            break;
        }
        if(FD_ISSET(sockfd,&fds)){
            bzero(buffer,sizeof(buffer));
            n=read(sockfd,buffer,sizeof(buffer));
            if(n<=0){
                printf("\nConnection closed by the server\n");
                break;
            }
            if(check_exit(buffer, password)==0){
        	printf("\nEXIT received from server. Closing session.\n");
        	break;
    		}
            write(1,buffer,n);
            if(strstr(buffer,"Your session has been started(server67)")!=NULL){
            		enable_raw();
	}
            else if(strstr(buffer,"Please enter password(server67): ")!=NULL){
                bzero(buffer,sizeof(buffer));
                n=read(0,buffer,sizeof(buffer));
                if(n<=0){
                    printf("Cant read password\n:");
                    break;
                }
                buffer[strcspn(buffer,"\n")]='\0';
                strncpy(password,buffer,sizeof(password)-1);
                password[sizeof(password)-1]='\0';
                if(write(sockfd,password,strlen(password))<0){
                    error("Write failed",1);
                    break;
                }
                skip_stdin = 1;
            }
        }
        if(!skip_stdin && FD_ISSET(0,&fds)){
            bzero(buffer,sizeof(buffer));
            n = read(0,buffer,sizeof(buffer));
            if(n<=0){
                error("user->server input problem",1);
                break;
            }
            if(write(sockfd,buffer,n)<0){
                error("Write failed",1);
                break;
            }
        }
        
    }
    disable_raw();
    close(sockfd);
    return;

}


int check_exit(char *buffer,char *password){
    const char *exit_msg = strstr(buffer, "EXIT|");
    if(exit_msg == NULL){
        return 1;
    }
    int random_number = 0;
    char received_hash[65] = {0};
    if(sscanf(exit_msg, "EXIT|%d|%64s", &random_number, received_hash) != 2){
        return 1;
    }
    char data[256];
    snprintf(data, sizeof(data), "EXIT%d%s", random_number, password);
    char computed_hash[65];
    sha256_cl(data,computed_hash);
    if(strcmp(received_hash,computed_hash)==0){
        return 0;}
    return 1;}

