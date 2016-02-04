//
//  SPTGuestController.cpp
//  SpatchServer
//
//  Created by adrienl on 20/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTGuestController.hpp"
#include <iostream>
#include <ctype.h>
#include <libssh/server.h>
#include "SPTTools.hpp"

SPTGuestController::SPTGuestController(const ssh_session * const session):
_session(session){

}

SPTGuestController::~SPTGuestController(){
    if (_channel != NULL){
        ssh_channel_close(_channel);
        ssh_channel_free(_channel);
    }
    std::cout << "SPTServGuest closed" << std::endl;
}

int matchPassword(const char *user, const char *password){
    return 1;
}

ssh_channel SPTGuestController::getChannel(){
    return _channel;
}

void SPTGuestController::_showMainMessage(){
    const std::string msg = std::string("Which server do you want to connect to\r\n");
    const std::string msg2 = std::string("(1) : Main server\r\n");
    ssh_channel_write(_channel, msg.c_str(), (uint32_t)msg.length());
    ssh_channel_write(_channel, msg2.c_str(), (uint32_t)msg2.length());
}

int SPTGuestController::init(){
    if (ssh_handle_key_exchange(*_session) != SSH_OK) {
        std::cerr << "ssh_handle_key_exchange: " << ssh_get_error(*_session) << std::endl;
        return -1;
    }
    
    if (SPTTools::instance().waitAuthentication(*_session, &matchPassword) < 0){
        std::cerr << "User not authenticated" << std::endl;
        return -1;
    }else{
        std::cout << "User authenticated" << std::endl;
    }
    _channel = SPTTools::instance().waitChannel(*_session);
    if (_channel == NULL){
        std::cerr << "Channel error" << std::endl;
        return -1;
    }else{
        std::cout << "Channel ready" << std::endl;
    }
    
    if (SPTTools::instance().waitShell(*_session) < 0){
        std::cerr << "Shell error" << std::endl;
        return -1;
    }else{
        std::cout << "Shell ready" << std::endl;
    }
    return 0;
}

int SPTGuestController::write(std::string message){
    
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
        std::cerr << "Remote channel write error : " << ssh_get_error(*_session) << std::endl;
        return -1;
    }
    
    return 0;
}

void SPTGuestController::_readSelectedServer(){
    char buff[2];
    int i = 0;
    do{
        i=ssh_channel_read(_channel,buff, 3, 0);
        buff[1] = 0;
        if(i>0) {
            std::string msg;
            if (isdigit(buff[0])){
                msg = std::string("You have selected server ") + std::string(&buff[0]) + "\n\r";
                msg = msg + std::string("Connection...\n\r");
                ssh_channel_write(_channel, msg.c_str(), (uint32_t)msg.length());
                break;
            }else{
                std::string msg = std::string("Server ") + std::string(&buff[0]) + std::string(" does not exists") + "\n\r";
                ssh_channel_write_stderr(_channel, msg.c_str(), (uint32_t)msg.length());
                this->_showMainMessage();
            }
            msg = "";
        }
    } while (i>0);
}

int SPTGuestController::showAvailableServer(std::string & host, int & port){
    
    if (_channel == NULL){
        std::cerr << "Channel is NULL" << std::endl;
        return -1;
    }
    
    this->_showMainMessage();
    
    this->_readSelectedServer();
    
    
    return 0;
}
