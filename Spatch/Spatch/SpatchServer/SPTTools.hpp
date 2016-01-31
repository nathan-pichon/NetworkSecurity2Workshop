//
//  SPTTools.h
//  SpatchServer
//
//  Created by adrienl on 31/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#ifndef SPTTools_h
#define SPTTools_h

#include <stdio.h>

typedef int (*SPTToolsPwdFunc)(const char*username, const char*password);

class SPTTools{
private:
    SPTTools(SPTTools const&){};
    SPTTools(){};
    ~SPTTools(){};
    static SPTTools * _instance;
    
public:
    static SPTTools &   instance();
    int                 verify_knownhost(ssh_session session);
    void                getClientIp(ssh_session session, std::string & host, int & port);
    ssh_channel         waitChannel(ssh_session session);
    int                 waitShell(ssh_session session);
    int                 waitAuthentication(ssh_session session, SPTToolsPwdFunc pwdValidationFunc);
};

#endif /* SPTTools_h */
