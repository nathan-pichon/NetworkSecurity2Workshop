//
//  SSHSessionManager.cpp
//  NetworkSecurity2Workshop
//
//  Created by adrienl on 10/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include <iostream>
#include "SSHSessionManager.hpp"

SSHSessionManager::SSHSessionManager(){
    _session = new ssh::Session;
}

SSHSessionManager::~SSHSessionManager(){
    std::cout << "Free Network Security" << std::endl;
    delete _session;
}

void SSHSessionManager::setHost(std::string host){
    _host = host;
    _session->setOption(SSH_OPTIONS_HOST, _host.c_str());
}

void SSHSessionManager::setPort(int port){
    _port = port;
    _session->setOption(SSH_OPTIONS_PORT, _port);
}

void SSHSessionManager::setVerbosity(NETSEC_LOG_VERBODITY verbosity){
    _verbosity = verbosity;
    _session->setOption(SSH_OPTIONS_LOG_VERBOSITY, _verbosity);
}

void SSHSessionManager::setOption(ssh_options_e option, std::string value){
    _session->setOption(option, value.c_str());
}

void SSHSessionManager::setOption(ssh_options_e option, int value){
    _session->setOption(option, value);
}

void SSHSessionManager::connect(){
    _session->connect();
}

void SSHSessionManager::disconnect(){
    _session->disconnect();
}

