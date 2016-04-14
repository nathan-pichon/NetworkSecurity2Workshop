//
//  main.cpp
//  Spatch
//
//  Created by adrienl on 14/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include <libssh/libssh.h>
#include "SPTServer.hpp"

#include <stdio.h>
#include <security/pam_appl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

int main(int argc, char **argv){
    
    ssh_session session = ssh_new();
    
    SPTServer server = SPTServer(session);
    
    server.start();
    
    ssh_free(session);
    
    std::cout << "SPATCH " << getpid() << " says : The End" << std::endl;
    
    return 0;
}