//
//  SPTGuestController.hpp
//  SpatchServer
//
//  Created by adrienl on 20/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTGuestController_hpp
#define SPTGuestController_hpp

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <iostream>

class SPTGuestController{

public:
    SPTGuestController(const ssh_session * const session);
    ~SPTGuestController();
    int showAvailableServer(std::string & host, int & port);
    int write(std::string message);
    int init();
    ssh_channel getChannel();
    
private:
    ssh_channel                 _channel;
    const ssh_session * const   _session;
    void                        _showMainMessage();
    void                        _readSelectedServer();
};

#endif /* SPTGuestController_hpp */
