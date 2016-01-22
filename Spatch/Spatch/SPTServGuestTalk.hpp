//
//  SPTServGuestTalk.hpp
//  SpatchServer
//
//  Created by adrienl on 20/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTServGuestTalk_hpp
#define SPTServGuestTalk_hpp

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <iostream>

class SPTServGuestTalk{

public:
    SPTServGuestTalk(const ssh_channel * const channel);
    ~SPTServGuestTalk();
    int start(std::string & host, int & port);
    
private:
    const ssh_channel * const _channel;
    void _showMainMessage();
};

#endif /* SPTServGuestTalk_hpp */
