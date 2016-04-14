//
//  SPTServerAuthentication.cpp
//  Spatch
//
//  Created by System Administrator on 1/14/16.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTServerAuthentication.hpp"
#include <libssh/server.h>
#include <iostream>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/openpam.h>
#include <sys/param.h>
#include <pwd.h>

SPTServerAuthentication::SPTServerAuthentication(const ssh_session session):
    _session(session)
{

}

SPTServerAuthentication::~SPTServerAuthentication(){

}

struct pam_response reply;

int pamPasswordCatch(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr){
    std::cout << "check" << std::endl;
    *resp = &reply;
    return PAM_SUCCESS;
}

int pamCompare(std::string user, std::string password){
    const struct pam_conv pamc = { pamPasswordCatch, NULL };
    pam_handle_t *pamh = NULL;
    int pam_err;
    
    if ((pam_err = pam_start("passwd", user.c_str(), &pamc, &pamh))){
        std::cout << "SPATCH says : Error : pam_start" << std::endl;
        return -1;
    }
    
    /* authenticate the applicant */
    if ((pam_err = pam_authenticate(pamh, 0)) != PAM_SUCCESS){
        std::cout << "SPATCH says : Error : pam_authenticate" << std::endl;
        return -1;
    }
    
    reply.resp = strdup(password.c_str());
    reply.resp_retcode = 0;
    pam_err = pam_authenticate(pamh, 0);
    
    if (pam_err != PAM_SUCCESS){
        if (pam_err == PAM_AUTH_ERR){
            std::cerr << "Authentication failure." << std::endl;
        }
        else{
            std::cerr << "pam_authenticate returned " << pam_err << std::endl;
        }
        return -1;
    }
    
    std::cout << "Authenticated" << std::endl;
    pam_err = pam_end(pamh, pam_err);
    free(reply.resp);
    if (pam_err != PAM_SUCCESS)
    {
        printf("pam_end returned\n");
        return -1;
    }
    
    return 0;
}

int authPassword(const ssh_message message){
    
    std::cout <<
    "User " <<
    ssh_message_auth_user(message) <<
    " wants to auth with pass " <<
    ssh_message_auth_password(message) <<
    std::endl;
    
    if(pamCompare(std::string(ssh_message_auth_user(message)),
                           std::string(ssh_message_auth_password(message))) == 0){
        ssh_message_auth_reply_success(message,0);
        ssh_message_free(message);
        return 0;
    }
    
    ssh_message_auth_set_methods(message,
                                 SSH_AUTH_METHOD_PASSWORD |
                                 SSH_AUTH_METHOD_INTERACTIVE);
    // not authenticated, send default message
    ssh_message_reply_default(message);
    
    return -1;
}

int SPTServerAuthentication::authenticate() {
    ssh_message message;
    
    while(1) {
        message=ssh_message_get(_session);
        if(!message)
            break;
        switch(ssh_message_type(message)){
            case SSH_REQUEST_AUTH:
                switch(ssh_message_subtype(message)){
                    case SSH_AUTH_METHOD_PASSWORD:
                        std::cout << SSH_AUTH_METHOD_PASSWORD << std::endl;
                        if (authPassword(message) == 0){
                            return 0;
                        }
                        break;
                    case SSH_AUTH_METHOD_NONE:
                    default:
                        std::cout <<
                        "User " <<
                        ssh_message_auth_user(message) <<
                        " wants to auth with unknown auth " <<
                        ssh_message_subtype(message) <<
                        std::endl;
                        
                        ssh_message_auth_set_methods(message,
                                                     SSH_AUTH_METHOD_PASSWORD |
                                                     SSH_AUTH_METHOD_INTERACTIVE);
                        ssh_message_reply_default(message);
                        break;
                }
                break;
            default:
                ssh_message_auth_set_methods(message,
                                             SSH_AUTH_METHOD_PASSWORD |
                                             SSH_AUTH_METHOD_INTERACTIVE);
                ssh_message_reply_default(message);
        }
        ssh_message_free(message);
    }
    return -1;
}
