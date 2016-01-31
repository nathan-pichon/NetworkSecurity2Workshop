//
//  SPTTools.c
//  SpatchServer
//
//  Created by adrienl on 31/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <sys/errno.h>
#include "SPTTools.hpp"

SPTTools* SPTTools::_instance = NULL;

SPTTools & SPTTools::instance(){
    if (_instance != NULL){
        _instance = new SPTTools();
    }
    return *_instance;
}


void SPTTools::getClientIp(ssh_session session, std::string & host, int & port) {
    struct sockaddr_storage tmp;
    struct sockaddr_in *sock;
    unsigned int len = 100;
    char ip[100] = "\0";
    getpeername(ssh_get_fd(session), (struct sockaddr*)&tmp, &len);
    sock = (struct sockaddr_in *)&tmp;
    inet_ntop(AF_INET, &sock->sin_addr, ip, len);
    port = sock->sin_port;
    std::string ip_str = ip;
    host = ip_str;
}

ssh_channel SPTTools::waitChannel(ssh_session session){
    ssh_message message;
    ssh_channel channel = NULL;
    
    do {
        message = ssh_message_get(session);
        if(message){
            if(ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN &&
               ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
                channel = ssh_message_channel_request_open_reply_accept(message);
                ssh_message_free(message);
                break;
            } else {
                ssh_message_reply_default(message);
                ssh_message_free(message);
            }
        } else {
            break;
        }
    } while(!channel);
    
    if(!channel) {
        std::cerr << "Error: cleint did not ask for a channel session" << ssh_get_error(session) << std::endl;
        ssh_finalize();
        return NULL;
    }
    
    return channel;
}

int SPTTools::waitAuthentication(ssh_session session, SPTToolsPwdFunc pwdValidationFunc){
    ssh_message message;
    do {
        message=ssh_message_get(session);
        if(!message)
            break;
        switch(ssh_message_type(message)){
            case SSH_REQUEST_AUTH:
                switch(ssh_message_subtype(message)){
                    case SSH_AUTH_METHOD_PASSWORD:
                        std::cout << "User " << ssh_message_auth_user(message) << " wants to auth with pass " <<
                        ssh_message_auth_password(message) << std::endl;
                        if ((*pwdValidationFunc)(ssh_message_auth_user(message), ssh_message_auth_password(message))){
                                ssh_message_auth_reply_success(message,0);
                                ssh_message_free(message);
                                return 1;
                        }
                        
                        ssh_message_auth_set_methods(message,
                                                     SSH_AUTH_METHOD_PASSWORD |
                                                     SSH_AUTH_METHOD_INTERACTIVE);
                        // not authenticated, send default message
                        ssh_message_reply_default(message);
                        break;
                        
                    case SSH_AUTH_METHOD_NONE:
                    default:
                        std::cout << "User " << ssh_message_auth_user(message) << " wants to auth with unknown auth " <<
                        ssh_message_subtype(message) << std::endl;
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
    } while (1);
    return 0;
}

int SPTTools::waitShell(ssh_session session){
    
    int shell = 0;
    ssh_message message;
    /* wait for a shell */
    do {
        message = ssh_message_get(session);
        if(message != NULL) {
            
            if(ssh_message_type(message) == SSH_REQUEST_CHANNEL) {
                if(ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SHELL) {
                    shell = 1;
                    ssh_message_channel_request_reply_success(message);
                    ssh_message_free(message);
                    break;
                } else if(ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_PTY) {
                    ssh_message_channel_request_reply_success(message);
                    ssh_message_free(message);
                    continue;
                }
            }
            ssh_message_reply_default(message);
            ssh_message_free(message);
        } else {
            break;
        }
    } while(!shell);
    
    if(!shell) {
        std::cerr << "Error: No shell requested " << ssh_get_error(session) << std::endl;;
        return -1;
    }
    return 0;
}

int SPTTools::verify_knownhost(ssh_session session){
    char *hexa;
    int state;
    char buf[10];
    unsigned char *hash = NULL;
    size_t hlen;
    int rc;
    
    state=ssh_is_server_known(session);
    
    std::cout << "ssh_is_server_known : " << state  << std::endl;
    
    
    ssh_key srv_pubkey;
    
    rc = ssh_get_publickey(session, &srv_pubkey);
    if (rc < 0){
        return -1;
    }
    
    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA1,
                                &hash,
                                &hlen);
    if (srv_pubkey != NULL){
        ssh_key_free(srv_pubkey);
    }
    if (rc < 0) {
        std::cout << "ssh_get_publickey_hash failure" << rc << std::endl;
        return -1;
    }else{
        std::cout << "ssh_get_publickey_hash succeded" << std::endl;
    }
    switch(state){
        case SSH_SERVER_KNOWN_OK:
            break; /* ok */
        case SSH_SERVER_KNOWN_CHANGED:
            fprintf(stderr,"Host key for server changed : server's one is now :\n");
            ssh_print_hexa("Public key hash",hash, hlen);
            ssh_clean_pubkey_hash(&hash);
            fprintf(stderr,"For security reason, connection will be stopped\n");
            return -1;
        case SSH_SERVER_FOUND_OTHER:
            fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
            fprintf(stderr,"An attacker might change the default server key to confuse your client"
                    "into thinking the key does not exist\n"
                    "We advise you to rerun the client with -d or -r for more safety.\n");
            return -1;
        case SSH_SERVER_FILE_NOT_FOUND:
            fprintf(stderr,"Could not find known host file. If you accept the host key here,\n");
            fprintf(stderr,"the file will be automatically created.\n");
            /* fallback to SSH_SERVER_NOT_KNOWN behavior */
        case SSH_SERVER_NOT_KNOWN:
            hexa = ssh_get_hexa(hash, hlen);
            fprintf(stderr,"The server is unknown. Do you trust the host key ?\n");
            fprintf(stderr, "Public key hash: %s\n", hexa);
            ssh_string_free_char(hexa);
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                ssh_clean_pubkey_hash(&hash);
                return -1;
            }
            if(strncasecmp(buf,"yes",3)!=0){
                ssh_clean_pubkey_hash(&hash);
                return -1;
            }
            fprintf(stderr,"This new key will be written on disk for further usage. do you agree ?\n");
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                ssh_clean_pubkey_hash(&hash);
                return -1;
            }
            if(strncasecmp(buf,"yes",3)==0){
                if (ssh_write_knownhost(session) < 0) {
                    ssh_clean_pubkey_hash(&hash);
                    std::cerr << "error " << strerror(errno) << std::endl;
                    return -1;
                }
            }
            
            break;
        case SSH_SERVER_ERROR:
            ssh_clean_pubkey_hash(&hash);
            fprintf(stderr,"%s",ssh_get_error(session));
            return -1;
    }
    ssh_clean_pubkey_hash(&hash);
    
    return 0;
}