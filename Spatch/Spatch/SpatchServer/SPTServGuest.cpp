//
//  SPTServGuest.cpp
//  SpatchServer
//
//  Created by adrienl on 17/01/2016.
//  Copyright © 2016 adrienlouf. All rights reserved.
//

#include "SPTServGuest.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <poll.h>
#include <errno.h>

SPTServGuest::SPTServGuest(const ssh_session session):
_channel(NULL),
_session(session),
_guestTalk(SPTServGuestTalk(&_channel)){}

SPTServGuest::~SPTServGuest(){
    if (_channel != NULL){
        ssh_channel_close(_channel);
        ssh_channel_free(_channel);
    }
    std::cout << "SPTServGuest closed" << std::endl;
}

int matchPassword(const char *user, const char *password){
    return 1;
}

int SPTServGuest::_authUser(){
    ssh_message message;
    do {
        message=ssh_message_get(_session);
        if(!message)
            break;
        switch(ssh_message_type(message)){
            case SSH_REQUEST_AUTH:
                switch(ssh_message_subtype(message)){
                    case SSH_AUTH_METHOD_PASSWORD:
                        std::cout << "User " << ssh_message_auth_user(message) << " wants to auth with pass " <<
                        ssh_message_auth_password(message) << std::endl;
                        if(matchPassword(ssh_message_auth_user(message),
                                         ssh_message_auth_password(message))){
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

int SPTServGuest::_waitChannel(){
    
    ssh_message message;
    _channel = 0;
    
    do {
        message = ssh_message_get(_session);
        if(message){
            if(ssh_message_type(message) == SSH_REQUEST_CHANNEL_OPEN &&
               ssh_message_subtype(message) == SSH_CHANNEL_SESSION) {
                _channel = ssh_message_channel_request_open_reply_accept(message);
                ssh_message_free(message);
                break;
            } else {
                ssh_message_reply_default(message);
                ssh_message_free(message);
            }
        } else {
            break;
        }
    } while(!_channel);
    
    if(!_channel) {
        std::cerr << "Error: cleint did not ask for a channel session" << ssh_get_error(_session) << std::endl;
        ssh_finalize();
        return -1;
    }
    
    return 0;
}

int SPTServGuest::_waitShell(){
    
    int shell = 0;
    ssh_message message;
    /* wait for a shell */
    do {
        message = ssh_message_get(_session);
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
        std::cerr << "Error: No shell requested " << ssh_get_error(_session) << std::endl;;
        return -1;
    }
    return 0;
}

int SPTServGuest::_initConnection(){
    if (ssh_handle_key_exchange(_session) != SSH_OK) {
        std::cerr << "ssh_handle_key_exchange: " << ssh_get_error(_session) << std::endl;
        return -1;
    }
    if (this->_authUser() < 0){
        std::cerr << "User not authenticated" << std::endl;
        return -1;
    }else{
        std::cout << "User authenticated" << std::endl;
    }
    if (this->_waitChannel() < 0){
        std::cerr << "Channel error" << std::endl;
        return -1;
    }else{
        std::cout << "Channel ready" << std::endl;
    }
    if (this->_waitShell() < 0){
        std::cerr << "Shell error" << std::endl;
        return -1;
    }else{
        std::cout << "Shell ready" << std::endl;
    }
    return 0;
}

void getClientIp(ssh_session session, std::string & host, int & port) {
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

int verify_knownhost(ssh_session session){
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

//int interactive_shell_session(ssh_session session, ssh_channel channel)
//{
//    char buffer[256];
//    int nbytes, nwritten;
//    while (ssh_channel_is_open(channel) &&
//           !ssh_channel_is_eof(channel))
//    {
//        struct timeval timeout;
//        ssh_channel in_channels[2], out_channels[2];
//        fd_set fds;
//        int maxfd;
//        timeout.tv_sec = 30;
//        timeout.tv_usec = 0;
//        in_channels[0] = channel;
//        in_channels[1] = NULL;
//        FD_ZERO(&fds);
//        FD_SET(0, &fds);
//        FD_SET(ssh_get_fd(session), &fds);
//        maxfd = ssh_get_fd(session) + 1;
//        ssh_select(in_channels, out_channels, maxfd, &fds, &timeout);
//        if (out_channels[0] != NULL)
//        {
//            nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
//            if (nbytes < 0) return SSH_ERROR;
//            if (nbytes > 0)
//            {
//                nwritten = write(1, buffer, nbytes);
//                if (nwritten != nbytes) return SSH_ERROR;
//            }
//        }
//        if (FD_ISSET(0, &fds))
//        {
//            nbytes = read(0, buffer, sizeof(buffer));
//            if (nbytes < 0) return SSH_ERROR;
//            if (nbytes > 0)
//            {
//                nwritten = ssh_channel_write(channel, buffer, nbytes);
//                if (nbytes != nwritten) return SSH_ERROR;
//            }
//        }
//    }
//    return rc;
//}


int callServer(const ssh_channel clientChannel, const char * guestIp, int guestPort){
    
    ssh_session my_ssh_session;
    ssh_channel my_ssh_channel;
    ssh_channel my_ssh_channel2;
    int rc;
    char *password;
    // Open session and set options
    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL){
        std::cout << "session NULL" << std::endl;
        return -1;
    }else{
        std::cout << "SESSION OK" << std::endl;
    }
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "192.168.1.229");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT_STR, "22");
    if (ssh_connect(my_ssh_session) < 0){
        std::cout << ssh_get_error(my_ssh_session) << std::endl;
        return -1;
    }else{
        std::cout << "SSH_CONNECT OK" << std::endl;
    }
    if (ssh_userauth_password(my_ssh_session, "vagrant", "vagrant") < 0){
        std::cout << ssh_get_error(my_ssh_session) << std::endl;
        return -1;
    }else{
        std::cout << "ssh_userauth_password OK" << std::endl;
    }
    
    ssh_channel_open_session(my_ssh_channel);
    

    
    my_ssh_channel = ssh_channel_new(my_ssh_session);
    if (ssh_channel_open_forward(my_ssh_channel,
                                 "192.168.1.229", 22,
                                 "localhost", 5555) < 0){
        std::cout << ssh_get_error(my_ssh_session) << std::endl;
        return -1;
    }else{
        std::cout << "ssh_channel_open_forward OK" << std::endl;
    }
    
    std::cout << "open ? : " << ssh_channel_is_open(my_ssh_channel) << std::endl;
    
    my_ssh_channel2 = ssh_channel_new(my_ssh_session);
    std::cout << "my_ssh_channel : " << my_ssh_channel << std::endl;
    
        if (ssh_channel_open_reverse_forward(my_ssh_channel2,
                                      "localhost", 5555,
                                     guestIp, guestPort) < 0){
            std::cout << ssh_get_error(my_ssh_session) << std::endl;
            return -1;
        }else{
            std::cout << "ssh_channel_open_reverse_forward OK" << std::endl;
        }
    
    std::cout << "open ? : " << ssh_channel_is_open(my_ssh_channel2) << std::endl;
    
//    while (ssh_channel_is_open(my_ssh_channel) &&
//           !ssh_channel_is_eof(my_ssh_channel))
//    {
//        int read = 1;
//        char buff[2048];
//        std::cout << "read now" << std::endl;
//        while (read){
//            memset(buff, 0, 2048);
//            int nbytes;
//            nbytes = ssh_channel_read(my_ssh_channel, buff, sizeof(buff) - 1, 0);
//            if (nbytes < 0)
//                return SSH_ERROR;
//            if (nbytes > 0)
//                ssh_channel_write(clientChannel, buff, nbytes);
//            if (nbytes < sizeof(buff) - 1){
//                read = 0;
//            }
//        }
//        
//        int write = 1;
//        std::cout << "write now" << std::endl;
//        
//        std::string guestStr = "";
//        
//        while (write){
//            memset(buff, 0, 2048);
//            int nbytes;
//            nbytes = ssh_channel_read(clientChannel, buff, sizeof(buff) - 1, 0);
//            if (nbytes < 0)
//                return SSH_ERROR;
//            if (buff[0] != '\r'){
//                ssh_channel_write(clientChannel, &buff[0], 1);
//                std::cout << "Append with : " << buff << std::endl;
//                guestStr = guestStr + std::string(buff);
//            }else{
//                ssh_channel_write(clientChannel, "\n\r", 2);
//                std::cout << "End with : " << buff << std::endl;
//                guestStr = guestStr + std::string("\n\r");
//                write = 0;
//            }
//            std::cout << guestStr << std::endl;
//        }
//        ssh_channel_write(my_ssh_channel, guestStr.c_str(), guestStr.length());
//    }
    
    ssh_channel_free(my_ssh_channel);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return 0;
}


int SPTServGuest::start(){
    
    if (this->_initConnection() < 0){
        return -1;
    }
    
    std::string host = "";
    int port;
    
    if (this->_guestTalk.start(host, port) < 0){
        return -1;
    }
    
    getClientIp(_session, host, port);
    
    std::cout << host << " " << port << std::endl;
    
    callServer(_channel, host.c_str(), port);
    
    return 0;
}