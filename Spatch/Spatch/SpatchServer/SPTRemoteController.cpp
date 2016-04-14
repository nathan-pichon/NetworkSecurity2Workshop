//
//  SPTRemoteController.cpp
//  SpatchServer
//
//  Created by adrienl on 28/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include <thread>
#include <iostream>
#include "SPTRemoteController.hpp"
#include "SPTTools.hpp"

int SPTRemoteController::init(){
    
    char buff[2048];
    
    if (ssh_connect(_session) < 0){
        std::cerr << ssh_get_error(_session) << std::endl;
        return -1;
    }
    
    if (SPTTools::instance().verify_knownhost(_session)){
        std::cerr << "Unknow host" << std::endl;
        return -1;
    }
    
    int rc = ssh_userauth_password(_session, NULL, "vagrant");
    if (rc != SSH_AUTH_SUCCESS)
    {
        std::cerr << "Error authenticating with password: " << ssh_get_error(_session) << std::endl;
        return -1;
    }
    
    _channel = SPTTools::instance().newChannel(_session);
    if (_channel == NULL){
        return -1;
    }
    
    if (SPTTools::instance().requestSheelSession(_channel, 80, 24)){
        std::cerr << "SPTTools : requestSheelSession error" << std::endl;
        return -1;
    }
    
    if (ssh_channel_is_open(_channel) == 0){
        std::cerr << "Channel is not opened" << std::endl;
        return -1;
    }
    
    if (ssh_channel_is_eof(_channel)){
        std::cerr << "Channel is end of file" << std::endl;
        return -1;
    }
    
    return 0;
}

int SPTRemoteController::sendMessageRequest(std::string message){
    
    if (_channel == NULL){
        std::cerr << "Channel is null" << std::endl;
        return -1;
    }
    
    if (ssh_channel_is_open(_channel) == 0){
        std::cerr << "Channel is not opened" << std::endl;
        return -1;
    }
    
    if (ssh_channel_is_eof(_channel)){
        std::cerr << "Channel is end of file" << std::endl;
        return -1;
    }
    
    int nwritten = ssh_channel_write(_channel, message.c_str(), (unsigned int)message.length());
    if (nwritten < 0){
        std::cerr << "Remote channel write error : " << ssh_get_error(_session) << std::endl;
        return -1;
    }
    
    return 0;
}

ssh_session SPTRemoteController::getSession(){
    return _session;
}

ssh_channel SPTRemoteController::getChannel(){
    return _channel;
}

int SPTRemoteController::disconnect(){
    if (ssh_is_connected(_session) == 1){
        std::cerr << "SSH already disconnect" << std::endl;
        return -1;
    }
    ssh_disconnect(_session);
    return 0;
}