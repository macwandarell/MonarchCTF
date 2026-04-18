#include "server.h"
#include "client.h"
#include <stdio.h>
#include <signal.h>



int main(int argc,char *argv[]){
    signal(SIGPIPE,SIG_IGN);
    if(argc<2){
        printf("Usage: %s <1/0 for client/server>",argv[0]);
        return 1;
    }
    if(strcmp(argv[1],"0")==0){
        start_server();
    }
    else if(strcmp(argv[1],"1")==0){
        client_run();}
    return 0;
}