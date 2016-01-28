//
//  SPTServ.cpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTServ.hpp"
#include "SPTServGuest.hpp"
#include <iostream>
#include <string>

SPTServ::SPTServ(const char * host, const char * port, const char * sshHostKeysPath):
    _session(ssh_new()),
    _bind(ssh_bind_new()),
    _port(port),
    _host(host),
    _sshHostKeysPath(sshHostKeysPath){}

SPTServ::~SPTServ(){
    
    ssh_disconnect(_session);
    
    std::cout << "disconnect" << std::endl;
    
    if (_bind != NULL){
        ssh_bind_free(_bind);
        _bind = NULL;
    }
    if (_session != NULL){
        ssh_free(_session);
        _session = NULL;
    }
    
    ssh_finalize();
}

int SPTServ::_initServ(){
    
    int vbool = 1;
    ssh_bind_options_set(_bind,
                         SSH_BIND_OPTIONS_DSAKEY,
                         (std::string(_sshHostKeysPath)+std::string("ssh_host_dsa_key")).c_str());
    ssh_bind_options_set(_bind,
                         SSH_BIND_OPTIONS_RSAKEY,
                         (std::string(_sshHostKeysPath)+std::string("ssh_host_rsa_key")).c_str());
    ssh_bind_options_set(_bind,
                         SSH_BIND_OPTIONS_BINDPORT_STR,
                         _port);
    int ret = ssh_options_set(_session, SSH_OPTIONS_SSH2, &vbool);
    
    if(ssh_bind_listen(_bind)<0){
        std::cerr << "Error listening to socket: " << ssh_get_error(_bind) << std::endl;
        return -1;
    }
    return 0;
}

int SPTServ::_forkServ(){
    int pid = fork();
    
    if(pid == 0){
        SPTServGuest servGuest = SPTServGuest(_session);
        if (servGuest.start() < 0){
            return -1;
        }
    }else if(pid > 0){
    
    }
    std::cout << "fork done : " << pid << std::endl;
    return pid;
//    return 0;
}

int SPTServ::start(){
    if (this->_initServ() < 0){
        return -1;
    }
    
    while (1){
        if (ssh_bind_accept(_bind, _session) != SSH_OK){
            std::cerr << ssh_get_error(_bind) << std::endl;
            continue;
        }
        
        int ret = this->_forkServ();
        if (ret == 0){
            return 0;
        }else if (ret < 0){
            return -1;
        }
        
    }
    
    return 0;
}