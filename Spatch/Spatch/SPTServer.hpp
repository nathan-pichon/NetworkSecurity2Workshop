//
//  SPTServer.hpp
//  Spatch
//
//  Created by adrienl on 14/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTServer_hpp
#define SPTServer_hpp

#include <stdio.h>
#include <string>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include "SPTServerAuthentication.hpp"

class SPTServer{
private:
    
    //Properties
    std::string _keyFolder;
    ssh_bind _bind;
    const ssh_session _session;
    SPTServerAuthentication _authentification;
    
    //Functions
    int _init();
    
public:
    
    //Properties
    SPTServer(const ssh_session session);
    ~SPTServer();
    int start();

};

#endif /* SPTServer_hpp */
