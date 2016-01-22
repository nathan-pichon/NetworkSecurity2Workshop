//
//  SPTServGuest.hpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTServGuest_hpp
#define SPTServGuest_hpp

#include <stdio.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include "SPTServGuestTalk.hpp"

class SPTServGuest{

public:
    SPTServGuest(const ssh_session session);
    ~SPTServGuest();
    int    start();

private:
    const ssh_session   _session;
    ssh_channel         _channel;
    int                 _initConnection();
    int                 _waitChannel();
    int                 _waitShell();
    int                 _authUser();
    SPTServGuestTalk    _guestTalk;
    
};

#endif /* SPTServGuest_hpp */
