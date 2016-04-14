//
//  SPTRemoteController.hpp
//  SpatchServer
//
//  Created by adrienl on 28/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTRemoteController_hpp
#define SPTRemoteController_hpp

#include <stdio.h>
#include <string>
#include <libssh/libssh.h>

class SPTRemoteController{
private:
    std::string                 _ip;
    unsigned int                _port;
    ssh_session                 _session;
    ssh_channel                 _channel;
    void *                      _delegate;

    
public:
    SPTRemoteController(std::string ip, unsigned int port, void * delegate):
    _ip(ip), _port(port), _session(ssh_new()), _channel(NULL){
        ssh_options_set(_session, SSH_OPTIONS_HOST, _ip.c_str());
        ssh_options_set(_session, SSH_OPTIONS_PORT, &_port);
        ssh_options_set(_session, SSH_OPTIONS_USER, "vagrant");
    };
    SPTRemoteController(){
        if (ssh_is_connected(_session) == 0){
            ssh_disconnect(_session);
        }
        ssh_free(_session);
    };
    int init();
    int disconnect();
    bool isConnected();
    int sendMessageRequest(std::string message);
    ssh_channel getChannel();
    ssh_session getSession();
};

#endif /* SPTRemoteController_hpp */
