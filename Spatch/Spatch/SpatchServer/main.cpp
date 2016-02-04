
#include "SPTMain.hpp"

#define DEFAULT_PORT "23"
#define DEFAULT_HOST "localhost"
#define SSH_HOST_KEYS_PATH "/etc/ssh/"

int main(int argc, char * argv[]){
    
    SPTMain serv = SPTMain(DEFAULT_HOST, DEFAULT_PORT, SSH_HOST_KEYS_PATH);
    
    if (serv.start() < 0){
        return -1;
    }
    
    return 0;
}