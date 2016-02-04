//
//  SPTMain.hpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTMain_hpp
#define SPTMain_hpp

#include <libssh/libssh.h>
#include <libssh/server.h>

class SPTMain{

public:
    SPTMain(const char * host, const char * port, const char * sshHostKeysPath);
    ~SPTMain();
    int start();
    
private:
    //Properties
    const char *    _host;
    const char *    _port;
    const char *    _sshHostKeysPath;
    ssh_session     _session;
    ssh_bind        _bind;
    //Methods
    int             _initServ();
    int             _forkServ();
    int             _initConnection();
    int             _authUser();
    int             _initChannel(ssh_channel);
    int            _manageGuest();
};

#endif /* SPTMain_hpp */
