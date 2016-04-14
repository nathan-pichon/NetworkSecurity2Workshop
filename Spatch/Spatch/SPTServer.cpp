//
//  SPTServer.cpp
//  Spatch
//
//  Created by adrienl on 14/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTServer.hpp"
#include <iostream>

SPTServer::SPTServer(const ssh_session session):
    _bind(ssh_bind_new()),
    _keyFolder("/etc/ssh/"),
    _session(session),
    _authentification(session){}

SPTServer::~SPTServer(){
    std::cout << "SPATCH says : delete SPTServer" << std::endl;
    ssh_bind_free(_bind);
}

int SPTServer::_init(){
    if (ssh_bind_options_set(_bind,
                         SSH_BIND_OPTIONS_DSAKEY,
                         (_keyFolder + "ssh_host_dsa_key").c_str()) < 0){
        std::cerr << "SPATCH says : Error bind option : " << ssh_get_error(_bind) << std::endl;
        return -1;
    }
    if (ssh_bind_options_set(_bind,
                         SSH_BIND_OPTIONS_RSAKEY,
                             (_keyFolder + "ssh_host_rsa_key").c_str()) < 0){
        std::cerr << "SPATCH says : Error bind option : " << ssh_get_error(_bind) << std::endl;
        return -1;
    }
    return 0;
}

int SPTServer::start(){
    
    if (_init() < 0){
        return -1;
    }
    
    if(ssh_bind_listen(_bind) < 0){
        std::cerr << "SPATCH says : Error listening to socket : " << ssh_get_error(_bind) << std::endl;
        return -1;
    }else{
        std::cout << "SPATCH says : ssh listening (ssh_bind_listen catched)" << std::endl;
    }
        
    if(ssh_bind_accept(_bind, _session) != SSH_OK){
        std::cerr << "SPATCH says : error accepting a connection : " << ssh_get_error(_bind) << std::endl;
        return -1;
    }else{
        std::cout << "SPATCH says : connection accepted (ssh_bind_accept catched)" << std::endl;
    }
    
    if (ssh_handle_key_exchange(_session) != SSH_OK) {
        std::cerr << "SPATCH says : ssh_handle_key_exchange : " << ssh_get_error(_session) << std::endl;
        return -1;
    }else{
        std::cout << "SPATCH says : handled keys (ssh_handle_key_exchange catched)" << std::endl;
    }
    
    if (_authentification.authenticate()){
        std::cout << "SPATCH says : authentication refused (SPTAuthentication::authenticate())" << std::endl;
        return -1;
    }else{
        std::cout << "SPATCH says : authenticated (SPTAuthentication::authenticate())" << std::endl;
    }
    
    return 0;
}

