//
//  SPTServGuestTalk.cpp
//  SpatchServer
//
//  Created by adrienl on 20/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTServGuestTalk.hpp"
#include <iostream>
#include <ctype.h>

SPTServGuestTalk::SPTServGuestTalk(const ssh_channel * const channel):
_channel(channel){

}

SPTServGuestTalk::~SPTServGuestTalk(){

}

void SPTServGuestTalk::_showMainMessage(){
    const std::string msg = std::string("Which server do you want to connect to\r\n");
    const std::string msg2 = std::string("(1) : Main server\r\n");
    ssh_channel_write(*_channel, msg.c_str(), (uint32_t)msg.length());
    ssh_channel_write(*_channel, msg2.c_str(), (uint32_t)msg2.length());
}

int SPTServGuestTalk::start(std::string & host, int & port){
    
    if (_channel == NULL){
        std::cerr << "Channel is NULL" << std::endl;
        return -1;
    }
    
    this->_showMainMessage();
    
    int i;
    char buf[3];
    
    do{
        i=ssh_channel_read(*_channel,buf, 3, 0);
        if(i>0) {
            std::string msg = std::string("You have selected server ") + std::string(&buf[0]) + "\n\r";
            if (isdigit(buf[0])){
                msg = msg + std::string("Connection...\n\r");
                ssh_channel_write(*_channel, msg.c_str(), (uint32_t)msg.length());
                break;
            }else{
                msg = msg + std::string("Unknow server...\n\r");
                ssh_channel_write(*_channel, msg.c_str(), (uint32_t)msg.length());
                this->_showMainMessage();
            }
            msg = "";
        }
    } while (i>0);
    
    return 0;
}
