#include "custom_error.h"

void error(const char *msg,int type){
    if(type==0){
    perror(msg);
    exit(1);}
    else if (type==1){
        perror(msg);
    }
}