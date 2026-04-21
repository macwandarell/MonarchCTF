#include "server.h"

char *logo=
"                                                                      \n"
" ▄    ▄                                    █        ▄▄▄ ▄▄▄▄▄▄▄ ▄▄▄▄▄▄\n"
" ██  ██  ▄▄▄   ▄ ▄▄    ▄▄▄    ▄ ▄▄   ▄▄▄   █ ▄▄   ▄▀   ▀   █    █     \n"
" █ ██ █ █▀ ▀█  █▀  █  ▀   █   █▀  ▀ █▀  ▀  █▀  █  █        █    █▄▄▄▄▄\n"
" █ ▀▀ █ █   █  █   █  ▄▀▀▀█   █     █      █   █  █        █    █     \n"
" █    █ ▀█▄█▀  █   █  ▀▄▄▀█   █     ▀█▄▄▀  █   █   ▀▄▄▄▀   █    █     \n"
"                                                                      \n";



struct active_users active_user_list[max_clients];
pthread_mutex_t server_lock=PTHREAD_MUTEX_INITIALIZER;
int current_active_users=0;
volatile int running=1;
int sockfd;
//SHA-256 function for exit code
void sha256_sv(const char *input, char output[65]) {
    printf("[DEBUG] sha256_sv called with input: %s\n", input);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);
    printf("[DEBUG] SHA256 hash computed, input length: %lu\n", strlen(input));
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
    printf("[DEBUG] sha256_sv output: %s\n", output);
}

void handle_signal(int sig){
    printf("[DEBUG] handle_signal called with sig: %d\n", sig);
    if(sig==SIGINT){
        printf("[DEBUG] SIGINT received, shutting down server\n");
        printf("Shutting off\n");
        running=0;
        printf("[DEBUG] running flag set to 0\n");
        //mark all users logged out
        printf("[DEBUG] Logging out %d active users\n", current_active_users);
        for(int i=0;i<current_active_users;i++){
            printf("[DEBUG] Logging out user: %s\n", active_user_list[i].username);
            user_logout(active_user_list[i].username);
        }
        printf("[DEBUG] Closing socket fd: %d\n", sockfd);
        close(sockfd);
        printf("[DEBUG] Server shutdown complete\n");
    }
}

void *handle_client(void *arg){
    //have to add client handling logic here
    //firstly, server has to give a choicde to client to create a room or join an existing room
    //since its a team based ctf, what the server will first do is it will generate a room code
    //it will share that room code to the client and then other client can join the same room using that code
    printf("[DEBUG] handle_client thread started\n");
    int newsockfd=*(int*) arg;
    printf("[DEBUG] newsockfd: %d\n", newsockfd);
    int n;
    enum State state=AUTH;
    printf("[DEBUG] Initial state: AUTH\n");
    char buffer[server_buffer];
    char current_user[50];
    char current_user_password[50];
    int admin_logged_in=0;
    char room_code[11];
    printf("[DEBUG] Client handler initialized\n");
    while(state!=EXIT){
    printf("[DEBUG] Client loop - current state: %d\n", state);
    switch(state){
        case AUTH:
                printf("[DEBUG] Entering AUTH state\n");
                bzero(buffer,sizeof(buffer));
                strcpy(buffer,logo);
                strcat(buffer,"Welcome to MonarchCTF! Please enter 1 if you want to register or 2 if you want to login or 3 for submissions or 4 for points:\n");
                printf("[DEBUG] Sending welcome message to client\n");
                if(write(newsockfd,buffer,strlen(buffer))<0){
                    printf("[DEBUG] write() failed for welcome message\n");
                    error("Error writing to socket(welcome screen)",1);
                    state=EXIT;
                    break;
                }
                printf("[DEBUG] Welcome message sent successfully\n");
                bzero(buffer,sizeof(buffer));
                printf("[DEBUG] Waiting for client input (register/login choice)...\n");
                n=read(newsockfd,buffer,sizeof(buffer));
                printf("[DEBUG] read() returned %d bytes\n", n);
                if(n<=0){
                    printf("[DEBUG] read() failed or connection closed\n");
                    error("Error reading from socket(welcome screen)",1);
                    state=EXIT;
                    break;
                }
                buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
                printf("[DEBUG] Received choice: %s\n", buffer);
                if(strcmp(buffer,"1")!=0 && strcmp(buffer,"2")!=0&&strcmp(buffer,"3")!=0&&strcmp(buffer,"4")!=0){
                    printf("[DEBUG] Invalid choice received: %s\n", buffer);
                    error("Invalid option selected by client",1);
                    continue;
                }
                else if(strcmp(buffer,"1")==0){
                    printf("[DEBUG] Client chose REGISTER\n");
                    char username[50],password[50];
                    printf("[DEBUG] Registration: requesting username\n");
                    strcpy(buffer,"Please enter username:");
                    if(write(newsockfd,buffer,strlen(buffer))<0){
                    printf("[DEBUG] write() failed for username prompt\n");
                    error("Error writing to socket(registration screen)",1);
                    state=EXIT;
                    break;
                    }
                    printf("[DEBUG] Username prompt sent, waiting for input...\n");
                    n=read(newsockfd,buffer,sizeof(buffer));
                    printf("[DEBUG] read() returned %d bytes for username\n", n);
                    if(n<=0){
                    printf("[DEBUG] read() failed for username\n");
                    error("Error reading from socket(registration screen)",1);
                    state=EXIT;
                    break;
                    }
                    buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
                    strncpy(username,buffer,50);
                    printf("[DEBUG] Username received: %s\n", username);
                    printf("[DEBUG] Registration: requesting password\n");
                    strcpy(buffer,"Please enter password(server67): ");
                    if(write(newsockfd,buffer,strlen(buffer))<0){
                    printf("[DEBUG] write() failed for password prompt\n");
                    error("Error writing to socket(registration screen)",1);
                    state=EXIT;
                    break;}
                    printf("[DEBUG] Password prompt sent, waiting for input...\n");
                    n=read(newsockfd,buffer,sizeof(buffer));
                    printf("[DEBUG] read() returned %d bytes for password\n", n);
                    if(n<=0){
                    printf("[DEBUG] read() failed for password\n");
                    error("Error reading from socket(registration screen)",1);
                    state=EXIT;
                    break;
                    }
                    buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
                    strncpy(password,buffer,50);
                    printf("[DEBUG] Password received (length: %lu)\n", strlen(password));
                    printf("[DEBUG] Attempting to create user: %s\n", username);
                    if(make_user(username,password)){
                        printf("[DEBUG] make_user() failed for %s\n", username);
                        error("Error creating user",1);
                        bzero(buffer,sizeof(buffer));
                        strcpy(buffer,"Username already exists or invalid attempt. Please try again.\n");
                        if(write(newsockfd,buffer,strlen(buffer))<0){
                            printf("[DEBUG] write() failed for registration failure message\n");
                            error("Error writing to socket(registration failure screen)",1);
                            state=EXIT;
                            break;
                        }
                        printf("[DEBUG] Continuing to AUTH state (registration failed)\n");
                        continue;
                    }
                    printf("[DEBUG] User %s created successfully\n", username);
                    bzero(buffer,sizeof(buffer));
                    strncpy(buffer,"User created successfully. Please login to continue.\n",strlen("User created successfully. Please login to continue.\n"));
                    printf("[DEBUG] Sending registration success message\n");
                    if(write(newsockfd,buffer,strlen(buffer))<0){
                    printf("[DEBUG] write() failed for success message\n");
                    error("Error writing to socket(login to continue screen)",1);
                    state=EXIT;
                    break;}
                    printf("[DEBUG] Returning to AUTH state for login\n");
                    continue;
                }
                else if(strcmp(buffer,"2")==0){
                        printf("[DEBUG] Client chose LOGIN\n");
                        char username[50],password[50];
                        printf("[DEBUG] Login: requesting username\n");
                        strcpy(buffer,"Please enter username:");
                        if(write(newsockfd,buffer,strlen(buffer))<0){
                        printf("[DEBUG] write() failed for login username prompt\n");
                        error("Error writing to socket(login screen)",1);
                        state=EXIT;
                        break;}
                        printf("[DEBUG] Username prompt sent, waiting for input...\n");
                        n=read(newsockfd,buffer,sizeof(buffer));
                        printf("[DEBUG] read() returned %d bytes for login username\n", n);
                        if(n<=0){
                        printf("[DEBUG] read() failed for login username\n");
                        error("Error reading from socket(login screen)",1);
                        state=EXIT;
                        break;}
                        buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
                        strncpy(username,buffer,50);
                        printf("[DEBUG] Login username: %s\n", username);
                        printf("[DEBUG] Login: requesting password\n");
                        strcpy(buffer,"Please enter password(server67): ");
                        if(write(newsockfd,buffer,strlen(buffer))<0){
                        printf("[DEBUG] write() failed for login password prompt\n");
                        error("Error writing to socket(login screen)",1);
                        state=EXIT;
                        break;}
                        printf("[DEBUG] Password prompt sent, waiting for input...\n");
                        n=read(newsockfd,buffer,sizeof(buffer));
                        printf("[DEBUG] read() returned %d bytes for login password\n", n);
                        if(n<=0){
                        printf("[DEBUG] read() failed for login password\n");
                        error("Error reading from socket(login screen)",1);
                        state=EXIT;
                        break;}
                        buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
                        strncpy(password,buffer,50);
                        printf("[DEBUG] Login password received (length: %lu)\n", strlen(password));
                        printf("[DEBUG] Checking credentials for user: %s\n", username);
                        int valid=user_check(username,password);
                        printf("[DEBUG] user_check() returned: %d\n", valid);
                        if(valid==1){
                            printf("[DEBUG] Login failed: incorrect credentials or already logged in\n");
                            if(write(newsockfd,"Incorrect credentials or logged in already.Please try again.\n",strlen("Incorrect credentials or logged in already.Please try again.\n"))<0){
                                printf("[DEBUG] write() failed for incorrect credentials message\n");
                                error("Error writing to socket(incorrect credentials screen)",1);
                                state=EXIT;
                                break;
                            }
                            printf("[DEBUG] Returning to AUTH state\n");
                            continue;
                        }
                        else if(valid==2){
                            printf("[DEBUG] Admin login successful\n");
                            admin_logged_in=1;
                        }
                        printf("[DEBUG] Regular user login successful\n");
                        //correct credentials, set current user and move on
                        printf("[DEBUG] Setting current_user to: %s\n", username);
                        strncpy(current_user,username,50);
                        strncpy(current_user_password,password,50);
                        printf("[DEBUG] Locking server_lock for adding active user\n");
                        pthread_mutex_lock(&server_lock);
                        printf("[DEBUG] Current active users: %d\n", current_active_users);
                        if(current_active_users<max_clients){
                            printf("[DEBUG] Adding user to active list at index %d\n", current_active_users);
                            active_user_list[current_active_users].socket=newsockfd;
                            strcpy(active_user_list[current_active_users].username,current_user);
                            current_active_users++;
                            printf("[DEBUG] Active users now: %d\n", current_active_users);
                        }
                        else{
                        printf("[DEBUG] Max clients reached, rejecting connection\n");
                        error("Max clients reached.",1);
                            if(write(newsockfd,"Too much load on server.Try later\n",strlen("Too much load on server.Try later\n"))<0){
                                printf("[DEBUG] write() failed for max clients message\n");
                                error("Error writing to socket(max clients reached screen)",1);
                                state=EXIT;
                                break;
                            }state=EXIT;break;}
                            printf("[DEBUG] Unlocking server_lock\n");
                            pthread_mutex_unlock(&server_lock);
                            printf("[DEBUG] Transitioning to ROOM state\n");
                            state=ROOM;}
                            else if(strcmp(buffer,"3")==0){
                            	if(write(newsockfd,"Write your solution as(ignore the braces): (room_code:prob_no:solution:) :",strlen("Write your solution as(ignore the braces): (room_code:prob_no:solution:) :"))<0){
                            	printf("[DEBUG] write() failed for submission message\n");
                                error("Error writing to socket( submission creen)",1);
                                state=EXIT;
                                break;
                            	}
                            	n=read(newsockfd,buffer,sizeof(buffer));
		                printf("[DEBUG] read() returned %d bytes for submission\n", n);
		                if(n<=0){
		                printf("[DEBUG] read() failed for submission\n");
		                error("Error reading from socket(submission screen)",1);
		                state=EXIT;
		                break;}
		                buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
		                if(submit_handler(buffer,newsockfd)){
					if(write(newsockfd,"Please try again, server error occured\n",strlen("Please try again, server error occured\n"))<0){
                            	printf("[DEBUG] write() failed for submission message\n");
                                error("Error writing to socket(  submission screen)",1);
                                state=EXIT;
                                break;
                            	}				
					}
				state=EXIT;
				break;
                            }
                            else if(strcmp(buffer,"4")==0){
                            	if(write(newsockfd,"Write team code to see points:",strlen("Write team code to see points:"))<0){
                            	printf("[DEBUG] write() failed for points message\n");
                                error("Error writing to socket( points screen)",1);
                                state=EXIT;
                                break;
                            	}
                            	n=read(newsockfd,buffer,sizeof(buffer));
		                printf("[DEBUG] read() returned %d bytes for points\n", n);
		                if(n<=0){
		                printf("[DEBUG] read() failed for points\n");
		                error("Error reading from socket(points screen)",1);
		                state=EXIT;
		                break;}
		                buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
		                if(points_handler(newsockfd,buffer)){
					if(write(newsockfd,"Please try again, server error occured\n",strlen("Please try again, server error occured\n"))<0){
                            	printf("[DEBUG] write() failed for points message\n");
                                error("Error writing to socket(  points screen)",1);
                                state=EXIT;
                                break;
                            	}				
					}
				state=EXIT;
				break;
                            }
                            break;

            case ROOM:
                        printf("[DEBUG] Entering ROOM state, admin_logged_in: %d\n", admin_logged_in);
                        if(admin_logged_in){
                            printf("[DEBUG] Admin handler called for user: %s\n", current_user);
                            if(admin_handler(newsockfd)){
                                printf("[DEBUG] admin_handler() failed\n");
                                error("Error in admin handler",1);
                                state=EXIT;
                                break;
                            }
                            printf("[DEBUG] Admin handler completed, transitioning to LOGGED_EXIT\n");
                            state=LOGGED_EXIT;
                        }
                        else{
                        printf("[DEBUG] Regular user in ROOM state, requesting room action\n");
                        strcpy(buffer,"Logged in.\nPlease enter room code to join or type 'create' if you want to make your own room:");
                        printf("[DEBUG] Sending room options message\n");
                        if(write(newsockfd,buffer,strlen(buffer))<0){
                            printf("[DEBUG] write() failed for room options\n");
                            error("Error writing to socket",1);
                            state=EXIT;
                            break;
                        }
                        bzero(buffer,sizeof(buffer));
                        printf("[DEBUG] Waiting for room input (code or 'create')...\n");
                        n=read(newsockfd,buffer,sizeof(buffer));
                        printf("[DEBUG] read() returned %d bytes for room input\n", n);
                        if(n<=0){
                            printf("[DEBUG] read() failed for room input\n");
                            error("Error reading from socket",1);
                            state=EXIT;
                            break;
                        }
                        buffer[n]='\0';buffer[strcspn(buffer, "\r\n")] = '\0';
                        printf("[DEBUG] Room input received: %s\n", buffer);
                        if(strncmp(buffer,"create",6)==0){
                            printf("[DEBUG] User requested to create room\n");
                            int a=create_room(newsockfd,current_user,room_code);
                            if(a==1){
                                printf("[DEBUG] create_room() failed\n");
                                error("Error creating room",1);
                                state=EXIT;
                                break;
                            }
                            else if(a==2){
                                printf("[DEBUG] create_room() failed\n");
                                error("Error creating room",1);
                                state=LOGGED_EXIT;
                                break;
                            }
                            printf("[DEBUG] Room created successfully, transitioning to PLAY\n");
                            state=PLAY;
                            break;
                        }
                        else{
                            printf("[DEBUG] User requesting to join room with code: %s\n", buffer);
                            int a=join_room(newsockfd,buffer,current_user,room_code);
                            printf("[DEBUG] join_room() returned: %d\n", a);
                            if(a==1){
                                printf("[DEBUG] join_room() failed\n");
                                error("Error joining room",1);
                                state=EXIT;
                                break;
                            }
                            else if(a==2){
                                printf("[DEBUG] Room code not found, retrying...\n");
                                continue;
                            }
                            printf("[DEBUG] Joined room successfully, transitioning to PLAY\n");
                            state=PLAY;
                            break;
                        }}
                        break;
            case PLAY:
            		bzero(buffer,sizeof(buffer));
            		strcpy(buffer,"Your session has been started(server67)");
            		if(write(newsockfd,buffer,sizeof(buffer))<0){
            			error("Error in user write to socket",1);
            			state=EXIT;
            			break;}
                        printf("[DEBUG] Entering PLAY state for user: %s\n", current_user);
                        bzero(buffer,sizeof(buffer));
                        if(run_playground(newsockfd,room_code)){
                            printf("[DEBUG] Error in playground for :%s\n",current_user);
                            error("Error in playground",1);
                            state=EXIT;
                            break;
                        }
                        printf("[DEBUG] Transitioning to LOGGED_EXIT\n");
                        state=LOGGED_EXIT;
                        break;
            case LOGGED_EXIT:
                        printf("[DEBUG] Entering LOGGED_EXIT state for user: %s\n", current_user);
                        //to end the connection, what we will do is make a hash which is hash=sha256("EXIT"+random_number+user_password) and then send it to client
                        int random_number=rand()%1000000;
                        printf("[DEBUG] Generated random number: %d\n", random_number);
                        char exit_hash[65];
                        char data[server_buffer];
                        sprintf(data,"EXIT%d%s",random_number,current_user_password);
                        printf("[DEBUG] Computing exit hash\n");
                        sha256_sv(data,exit_hash);
                        char message[321];
                        sprintf(message,"EXIT|%d|%s",random_number,exit_hash);
                        printf("[DEBUG] Exit message prepared, sending to client\n");
                        if(write(newsockfd,message,strlen(message))<0){
                            printf("[DEBUG] write() failed for exit message\n");
                            error("Error writing exit code to socket",1);
                            state=EXIT;
                            break;
                        }
                        printf("[DEBUG] Calling user_logout for: %s\n", current_user);
                        user_logout(current_user);
                        printf("[DEBUG] Locking server_lock to remove from active list\n");
                        pthread_mutex_lock(&server_lock);
                        printf("[DEBUG] Current active users: %d\n", current_active_users);
                        for(int i=0;i<current_active_users;i++){
                            printf("[DEBUG] Checking active user %d: %s\n", i, active_user_list[i].username);
                            if(strcmp(active_user_list[i].username,current_user)==0){
                                printf("[DEBUG] Found user at index %d, removing...\n", i);
                                for(int j=i;j<current_active_users-1;j++){
                                    active_user_list[j]=active_user_list[j+1];
                                }
                                current_active_users--;
                                printf("[DEBUG] User removed, active users now: %d\n", current_active_users);
                        break;
                        }}
                        printf("[DEBUG] Unlocking server_lock\n");
                        pthread_mutex_unlock(&server_lock);
                        printf("[DEBUG] Transitioning to EXIT\n");
                        state=EXIT;
                        break;
                default:
                        printf("[DEBUG] Invalid state: %d\n", state);
                        error("Invalid state",1);
                        state=EXIT;
                        break;
            }}
    printf("[DEBUG] Exiting client handler loop\n");
    printf("[DEBUG] Closing newsockfd: %d\n", newsockfd);
    close(newsockfd);
    printf("[DEBUG] Freeing arg pointer\n");
    free(arg);
    printf("[DEBUG] Client handler thread ending\n");
    return NULL;
}

//start server and listen for new connection
void start_server(){
    printf("[DEBUG] start_server() called\n");
    signal(SIGINT,handle_signal);
    printf("[DEBUG] SIGINT handler registered\n");
    printf("Enter a port number to start the server: ");
    int portno;
    scanf("%d",&portno);
    printf("[DEBUG] Port number entered: %d\n", portno);
    printf("[DEBUG] Opening database files...\n");
    int room_file=open("room_codes",O_RDWR|O_CREAT,0666);
    printf("[DEBUG] room_codes fd: %d\n", room_file);
    if(room_file<0){
        printf("[DEBUG] Failed to open room_codes file\n");
        error("Error opening room codes file",0);
    }
    int user_file=open("users",O_RDWR|O_CREAT,0666);
    printf("[DEBUG] users fd: %d\n", user_file);
    if(user_file<0){
        printf("[DEBUG] Failed to open users file\n");
        error("Error opening users file",0);
    }
    int room_details_file=open("rooms",O_RDWR|O_CREAT,0666);
    printf("[DEBUG] rooms fd: %d\n", room_details_file);
    if(room_details_file<0){
        printf("[DEBUG] Failed to open rooms file\n");
        error("Error opening rooms file",0);
    }
    int solutions_file=open("solutions",O_RDWR|O_CREAT,0666);
    printf("[DEBUG] room_codes fd: %d\n", room_file);
    if(solutions_file<0){
        printf("[DEBUG] Failed to open solutions file\n");
        error("Error opening solutions file",0);
    }
    printf("[DEBUG] Closing file descriptors\n");
    close(room_file);
    close(user_file);
    close(room_details_file);
    close(solutions_file);
    printf("[DEBUG] Database files initialized\n");
    //not debugging or checking anything here, mkdir just returns -1 when directory already exists
    char path[256];
    snprintf(path,sizeof(path),"/home/ctf");
    char jail_path[300];
    snprintf(jail_path,sizeof(jail_path),"/home/ctf/jail");
    char bin_path[320];
    snprintf(bin_path,sizeof(bin_path),"%s/bin",jail_path);
    char lib_path[320];
    snprintf(lib_path,sizeof(lib_path),"%s/lib",jail_path);
    char lib64_path[320];
    snprintf(lib64_path,sizeof(lib64_path),"%s/lib64",jail_path);
    char problems_path[320];
    snprintf(problems_path,sizeof(problems_path),"%s/problems",jail_path);
    char usr_path[320];
    snprintf(usr_path,sizeof(usr_path),"%s/usr",jail_path);
    char share_path[340];
    snprintf(share_path,sizeof(share_path),"%s/share",usr_path);
    char terminfo_path[360];
    snprintf(terminfo_path,sizeof(terminfo_path),"%s/terminfo",share_path);
    char x_path[360];
    snprintf(x_path,sizeof(x_path),"%s/x",terminfo_path);
    char dev_path[360];
    snprintf(dev_path,sizeof(dev_path),"%s/dev",jail_path);
    char pts_path[380];
    snprintf(pts_path,sizeof(pts_path),"%s/dev",dev_path);
    mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(jail_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(bin_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(lib_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(lib64_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(problems_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(usr_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(share_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(terminfo_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(x_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(dev_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    mkdir(pts_path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    system("cp /usr/share/terminfo/x/xterm /home/ctf/jail/usr/share/terminfo/x/");
    system("cp /usr/share/terminfo/x/xterm-256color /home/ctf/jail/usr/share/terminfo/x/");
    //for the below you can also use mknod, but i am not doing that here because cp -a is just faster for me to do
    system("cp -a /dev/null /home/ctf/jail/dev/");
    system("cp -a /dev/tty /home/ctf/jail/dev/");
    system("cp -a /dev/urandom /home/ctf/jail/dev/");
    int *newsockfd,n;
    struct sockaddr_in server_addr,client_addr;
    pthread_t tid;
    socklen_t client_len;
    printf("[DEBUG] Creating socket...\n");
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    printf("[DEBUG] Socket created, fd: %d\n", sockfd);

    if(sockfd<0){
        printf("[DEBUG] socket() failed\n");
        error("Error opening socket.",0);
    }
    printf("[DEBUG] Socket creation successful\n");

    printf("[DEBUG] Setting up server address structure...\n");
    bzero((char *)&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(portno);
    printf("[DEBUG] Server address configured for port: %d\n", portno);

    printf("[DEBUG] Binding socket to address...\n");
    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        printf("[DEBUG] bind() failed\n");
        error("Error on bind",0);
    }
    printf("[DEBUG] Socket bound successfully\n");

    printf("[DEBUG] Listening for connections (max_clients: %d)...\n", max_clients);
    listen(sockfd,max_clients);
    printf("[DEBUG] Server listening on socket fd: %d\n", sockfd);

    printf("[DEBUG] Entering server accept loop\n");
    while(running){
        printf("[DEBUG] Waiting for incoming connection...\n");
        //using pthreads for multiple clients
        client_len=sizeof(client_addr);
        newsockfd=malloc(sizeof(int));
        printf("[DEBUG] Allocated memory for newsockfd\n");
        *newsockfd=accept(sockfd,(struct sockaddr *)&client_addr,&client_len);
        printf("[DEBUG] accept() returned fd: %d\n", *newsockfd);
        if(*newsockfd<0){
            printf("[DEBUG] accept() failed\n");
            error("Accept failed",1);
            continue;
        }
        printf("[DEBUG] New client connected, creating thread...\n");
        pthread_create(&tid,NULL,handle_client,newsockfd);
        printf("[DEBUG] Thread created with tid\n");
        pthread_detach(tid);
        printf("[DEBUG] Thread detached\n");
    }
    printf("[DEBUG] Exiting accept loop (running=0)\n");
    printf("[DEBUG] Closing server socket fd: %d\n", sockfd);
    close(sockfd);
    printf("[DEBUG] Server shutdown complete\n");
    return;
}


int create_room(int newsockfd,char *owner,char *room_code){
    printf("[DEBUG] create_room() called for owner: %s\n", owner);
    char code[11];
    printf("[DEBUG] Generating room code...\n");
    if(generate_room_code(code)){
        printf("[DEBUG] generate_room_code() failed\n");
        error("Error generating room code",1);
        return 1;
    }
    printf("[DEBUG] Room code generated: %s\n", code);
    printf("[DEBUG] Creating room in database...\n");
    if(make_room(code,owner)){
        printf("[DEBUG] owner already has a room");
        if(remove_room_code(code)){error("Error removing room code",1);return 1;}
        char *msg="Please login again and join your own room or use another id\n";
        if(write(newsockfd,msg,strlen(msg))<0){
            error("Error in writing in create room exists",1);
            return 1;
        }
        return 2;
    }
    printf("[DEBUG] Room created, sending code to client\n");
    char message[100];
    sprintf(message, "Room has been created, room code is: %s\n", code);
    if((write(newsockfd,message,strlen(message)))<0){
        printf("[DEBUG] write() failed for room code\n");
        error("Error writing room code",1);
        return 1;
    }
    printf("[DEBUG] Adding owner to room...\n");
    if(user_join_room(code,owner)){
        printf("[DEBUG] user_join_room() failed\n");
        return 1;
    }
    strncpy(room_code,code,11);
    room_code[10]='\0';
    printf("[DEBUG] create_room() completed successfully\n");
    return 0;
}

int generate_room_code(char *code){
    printf("[DEBUG] generate_room_code() called\n");
    //generate a code and check if it already exists
    //here we need to use locking as well because 2 clients can end up having the same code
    printf("[DEBUG] Locking room_lock\n");
    pthread_mutex_lock(&room_lock);
    printf("[DEBUG] room_lock acquired\n");

    int fd=open("room_codes",O_RDWR);
    printf("[DEBUG] room_codes fd: %d\n", fd);
    if(fd<0){
        printf("[DEBUG] Failed to open room_codes file\n");
        error("Error opening room codes file",1);
        pthread_mutex_unlock(&room_lock);
        printf("[DEBUG] room_lock released after error\n");
        return 1;
    }
    int exists;
    char room_code[11];
    room_code[10]='\0';
    int attempts=0;
    do{
        printf("[DEBUG] Room code generation attempt: %d\n", ++attempts);
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
        printf("[DEBUG] Generated code: %.10s\n", room_code);
        //check if code already exists
        lseek(fd,0,SEEK_SET);
        char buffer[10];
        int read_count=0;
        while(read(fd,buffer,sizeof(buffer))>0){
            read_count++;
            printf("[DEBUG] Checking against existing code #%d\n", read_count);
            if(strncmp(buffer,room_code,10)==0){
                printf("[DEBUG] Code already exists!\n");
                exists=1;
                break;
            }
        }
        if(!exists) printf("[DEBUG] Code is unique\n");
        }while(exists);
    
    //write code to the file
    printf("[DEBUG] Writing new room code to file\n");
    lseek(fd,0,SEEK_END);
    write(fd,room_code,10);
    memcpy(code, room_code, 10);
    code[10] = '\0';
    printf("[DEBUG] Room code stored: %s\n", code);
    close(fd);
    printf("[DEBUG] Unlocking room_lock\n");
    pthread_mutex_unlock(&room_lock);
    printf("[DEBUG] generate_room_code() completed\n");
    return 0;
}


int join_room(int newsockfd,char *buffer,char *current_user,char *room_code){
    printf("[DEBUG] join_room() called for user: %s\n", current_user);
    int fd=open("room_codes",O_RDONLY);
    printf("[DEBUG] room_codes fd: %d\n", fd);
    if(fd<0){
        printf("[DEBUG] Failed to open room_codes file\n");
        error("Error opening room codes file",1);
        return 1;
    }
    char code[11];
    strncpy(code,buffer,10);
    code[10]='\0';
    printf("[DEBUG] Looking for room code: %.10s\n", code);
    lseek(fd,0,SEEK_SET);
    int exists=0;
    char file_buffer[10];
    int check_count=0;
    while(read(fd,file_buffer,sizeof(file_buffer))>0){
        check_count++;
        printf("[DEBUG] Checking room code #%d\n", check_count);
        if(strncmp(file_buffer,code,10)==0){
            printf("[DEBUG] Room code found!\n");
            exists=1;
            break;
        }
    }
    close(fd);
    if(!exists){
        printf("[DEBUG] Room code not found\n");
        char *message="Room code not found. Please try again.";
        if(write(newsockfd,message,strlen(message))<0){
            printf("[DEBUG] write() failed for room not found message\n");
            error("Error writing to socket",1);
            return 1;
    }
    return 2;}
    printf("[DEBUG] Room code verified, adding user to room\n");
    //define what a room exactly is and then join
    //have to add more logic here
    if(user_join_room(code,current_user)){
        printf("[DEBUG] user_join_room() failed\n");
        char *message="Please login again and enter a correct room code or create a new room\n";
         if(write(newsockfd,message,strlen(message))<0){
            printf("[DEBUG] write() failed for room join fail message\n");
            error("Error writing to socket",1);
            return 1;
        }
        error("Error joining room",1);
        return 1;}
    printf("[DEBUG] User added to room, sending success message\n");
    char *message="Joined room successfully.";
    if(write(newsockfd,message,strlen(message))<0){
        printf("[DEBUG] write() failed for success message\n");
        error("Error writing to socket",1);
        return 1;
    }
    strncpy(room_code,code,11);
    room_code[10]='\0';
    printf("[DEBUG] join_room() completed successfully\n");
    return 0;
}

int remove_active_user(char *current_user){
    printf("[DEBUG] remove_active_user() called for: %s\n", current_user);
    printf("[DEBUG] Locking server_lock\n");
    pthread_mutex_lock(&server_lock);
    printf("[DEBUG] server_lock acquired, searching for user...\n");
    printf("[DEBUG] Current active users: %d\n", current_active_users);
    for(int i=0;i<current_active_users;i++){
        printf("[DEBUG] Checking user %d: %s\n", i, active_user_list[i].username);
        if(strcmp(active_user_list[i].username,current_user)==0){
            printf("[DEBUG] User found at index %d, removing...\n", i);
            int fd=active_user_list[i].socket;
            printf("[DEBUG] Sending kick message to fd: %d\n", fd);
            write(fd,"You have been Kicked out by server. You can exit now.Logging out...\n",strlen("You have been Kicked out by server. You can exit now.Logging out...\n"));
            printf("[DEBUG] Shutting down socket fd: %d\n", fd);
            shutdown(fd,SHUT_RDWR);
            printf("[DEBUG] Removing from active list\n");            
            for(int j=i;j<current_active_users-1;j++){
                active_user_list[j]=active_user_list[j+1];
            }
            current_active_users--;
            printf("[DEBUG] User removed, active users now: %d\n", current_active_users);
            break;
        }
    }
    printf("[DEBUG] Unlocking server_lock\n");
    pthread_mutex_unlock(&server_lock);
    printf("[DEBUG] remove_active_user() completed\n");
    return 0;
}


int remove_room_code(char *code){
    printf("[DEBUG] remove_room_code() called for code: %s\n", code);
    pthread_mutex_lock(&room_lock);
    printf("[DEBUG] room_lock acquired\n");
    int fd=open("room_codes",O_RDWR);
    printf("[DEBUG] room_codes fd: %d\n", fd);
    if(fd<0){
        printf("[DEBUG] Failed to open room_codes file\n");
        error("Error opening room codes file",1);
        pthread_mutex_unlock(&room_lock);
        printf("[DEBUG] room_lock released after error\n");
        return 1;
    }
    //seek to end and truncate by 10 bytes (one room code)
    lseek(fd,0,SEEK_END);
    off_t file_size=lseek(fd,0,SEEK_CUR);
    printf("[DEBUG] Current file size: %ld\n", file_size);
    if(file_size>=10){
        if(ftruncate(fd,file_size-10)<0){
            printf("[DEBUG] ftruncate() failed\n");
            error("Error truncating room codes file",1);
            close(fd);
            pthread_mutex_unlock(&room_lock);
            return 1;
        }
        printf("[DEBUG] File truncated, removed last room code\n");
    }
    close(fd);
    printf("[DEBUG] Unlocking room_lock\n");
    pthread_mutex_unlock(&room_lock);
    printf("[DEBUG] remove_room_code() completed\n");
    return 0;

}
    



