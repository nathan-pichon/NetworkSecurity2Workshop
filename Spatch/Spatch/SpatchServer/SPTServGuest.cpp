//
//  SPTServGuest.cpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTServGuest.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <poll.h>
#include <errno.h>
#include "SPTTools.hpp"

SPTServGuest::SPTServGuest(const ssh_session session):
_channel(NULL),
_session(session),
_guestTalk(SPTServGuestTalk(&_channel)){}

SPTServGuest::~SPTServGuest(){
    if (_channel != NULL){
        ssh_channel_close(_channel);
        ssh_channel_free(_channel);
    }
    std::cout << "SPTServGuest closed" << std::endl;
}

int matchPassword(const char *user, const char *password){
    return 1;
}

int SPTServGuest::_initConnection(){
    if (ssh_handle_key_exchange(_session) != SSH_OK) {
        std::cerr << "ssh_handle_key_exchange: " << ssh_get_error(_session) << std::endl;
        return -1;
    }

    if (SPTTools::instance().waitAuthentication(_session, &matchPassword) < 0){
        std::cerr << "User not authenticated" << std::endl;
        return -1;
    }else{
        std::cout << "User authenticated" << std::endl;
    }
    _channel = SPTTools::instance().waitChannel(_session);
    if (_channel == NULL){
        std::cerr << "Channel error" << std::endl;
        return -1;
    }else{
        std::cout << "Channel ready" << std::endl;
    }
    if (SPTTools::instance().waitShell(_session) < 0){
        std::cerr << "Shell error" << std::endl;
        return -1;
    }else{
        std::cout << "Shell ready" << std::endl;
    }
    return 0;
}


int SPTServGuest::start(){
    
    if (this->_initConnection() < 0){
        return -1;
    }
    
    std::string host = "";
    int port;
    
    if (this->_guestTalk.start(host, port) < 0){
        return -1;
    }
    
    SPTTools::instance().getClientIp(_session, host, port);

    std::cout << host << " " << port << std::endl;
    
    return 0;
}