//
//  SPTBinder.hpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTBinder_hpp
#define SPTBinder_hpp

#include <stdio.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include "SPTGuestController.hpp"
#include "SPTRemoteController.hpp"

typedef enum SPTBinderTarget{
    ETargetClient,
    ETargetServer
}SPTBinderTarget;

class SPTBinder{

public:
    SPTBinder(const ssh_session session);
    ~SPTBinder();
    int     start();
    int     write(std::string, SPTBinderTarget target);

private:
    const ssh_session   _session;
    SPTGuestController  _guestController;
};

#endif /* SPTBinder_hpp */
