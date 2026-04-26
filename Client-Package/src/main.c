#include "client.h"
#include <stdio.h>
#include <signal.h>



int main(int argc,char *argv[]){
    signal(SIGPIPE,SIG_IGN);
    client_run();
    return 0;
}
