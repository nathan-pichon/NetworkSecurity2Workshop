//
//  SPTBinder.cpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTBinder.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <poll.h>
#include <errno.h>
#include "SPTTools.hpp"

SPTBinder::SPTBinder(const ssh_session session):
_session(session),
_guestController(SPTGuestController(&_session)){}

SPTBinder::~SPTBinder(){

}

int SPTBinder::start(){
    
    std::string host = "";
    int port;
    
    if (_guestController.init() < 0){
        std::cerr << "Guest controller init error" << std::endl;
        return -1;
    }
    
    if (_guestController.showAvailableServer(host, port) < 0){
        return -1;
    }
    
    
    SPTRemoteController remoteController = SPTRemoteController("192.168.1.229", 22, this);
    remoteController.init();
    ssh_channel remoteChannel = remoteController.getChannel();
    ssh_session remoteSession = remoteController.getSession();
    ssh_channel clientChannel = _guestController.getChannel();
    
    SPTTools::instance().interactive_shell_session(_session, clientChannel, remoteSession, remoteChannel);
    
    return 0;
}