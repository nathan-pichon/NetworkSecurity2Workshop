//
//  SPTServerAuthentication.hpp
//  Spatch
//
//  Created by System Administrator on 1/14/16.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTServerAuthentication_hpp
#define SPTServerAuthentication_hpp

#include <stdio.h>
#include <string>
#include <libssh/libssh.h>

class SPTServerAuthentication{
public:
    SPTServerAuthentication(const ssh_session);
    ~SPTServerAuthentication();
    int authenticate();
private:
    const ssh_session _session;
};

#endif /* SPTServerAuthentication_hpp */
