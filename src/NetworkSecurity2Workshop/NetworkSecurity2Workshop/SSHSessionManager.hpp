//
//  SSHSessionManager.hpp
//  NetworkSecurity2Workshop
//
//  Created by adrienl on 10/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SSHSessionManager_hpp
#define SSHSessionManager_hpp

#include <stdio.h>
#include <string>
#include <libssh/libsshpp.hpp>

class SSHSessionManager{

private:
    typedef enum  {
        NETSEC_LOG_NOLOG = 0,
        NETSEC_LOG_WARNING = 1,
        NETSEC_LOG_PROTOCOL = 2,
        NETSEC_LOG_PACKET = 3,
        NETSEC_LOG_FUNCTIONS = 4,
    } NETSEC_LOG_VERBODITY;
    
    ssh::Session *          _session;
    std::string             _host = "";
    int                     _port = -1;
    NETSEC_LOG_VERBODITY    _verbosity = NETSEC_LOG_NOLOG;
    
public:
    //Function
    SSHSessionManager();
    ~SSHSessionManager();
    void setOption(ssh_options_e option, std::string value);
    void setOption(ssh_options_e option, int value);
    void connect();
    void disconnect();
    void setPort(int port);
    void setHost(std::string host);
    void setVerbosity(NETSEC_LOG_VERBODITY);
    //Properties
};

#endif /* SSHSessionManager_hpp */
