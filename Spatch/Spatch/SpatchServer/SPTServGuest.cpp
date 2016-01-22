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

//static int copy_fd_to_chan(socket_t fd, int revents, void *userdata) {
//    ssh_channel chan = (ssh_channel)userdata;
//    char buf[2048];
//    int sz = 0;
//    
//    if(!chan) {
//        close(fd);
//        return -1;
//    }
//    if(revents & POLLIN) {
//        sz = read(fd, buf, 2048);
//        if(sz > 0) {
//            ssh_channel_write(chan, buf, sz);
//        }
//    }
//    if(revents & POLLHUP) {
//        ssh_channel_close(chan);
//        sz = -1;
//    }
//    return sz;
//}
//
//static int copy_chan_to_fd(ssh_session session,
//                           ssh_channel channel,
//                           void *data,
//                           uint32_t len,
//                           int is_stderr,
//                           void *userdata) {
//    int fd = *(int*)userdata;
//    int sz;
//    (void)session;
//    (void)channel;
//    (void)is_stderr;
//    
//    sz = write(fd, data, len);
//    return sz;
//}
//
//static void chan_close(ssh_session session, ssh_channel channel, void *userdata) {
//    int fd = *(int*)userdata;
//    (void)session;
//    (void)channel;
//    
//    close(fd);
//}
//
//
//
//struct ssh_channel_callbacks_struct cb = {
//    .channel_data_function = copy_chan_to_fd,
//    .channel_eof_function = chan_close,
//    .channel_close_function = chan_close,
//    .userdata = NULL
//};

//int doSelect(ssh_session session, const ssh_channel forwardingChannel){
//    char buffer[256];
//    int nbytes, nwritten;
//    std::cout << "ssh_channel_is_closed : " << ssh_channel_is_closed(forwardingChannel) << std::endl;
//    std::cout << "ssh_channel_is_open : " << ssh_channel_is_open(forwardingChannel) << std::endl;
//    std::cout << "ssh_channel_is_eof : " << ssh_channel_is_eof(forwardingChannel) << std::endl;
//    while (ssh_channel_is_open(forwardingChannel) &&
//           !ssh_channel_is_eof(forwardingChannel))
//    {
//        std::cout << "doSelect while" << std::endl;
//        struct timeval timeout;
//        ssh_channel in_channels[2], out_channels[2];
//        fd_set fds;
//        int maxfd;
//        timeout.tv_sec = 30;
//        timeout.tv_usec = 0;
//        in_channels[0] = forwardingChannel;
//        in_channels[1] = NULL;
//        FD_ZERO(&fds);
//        FD_SET(ssh_get_fd(session), &fds);
//        maxfd = ssh_get_fd(session) + 1;
//        ssh_select(in_channels, out_channels, maxfd, &fds, &timeout);
//        if (out_channels[0] != NULL)
//        {
//            nbytes = ssh_channel_read(forwardingChannel, buffer, sizeof(buffer), 0);
//            if (nbytes < 0) return -1;
//            if (nbytes > 0)
//            {
//                nwritten = ssh_channel_write(forwardingChannel, buffer, nbytes);
//                if (nbytes != nwritten) return -1;
//            }
//        }
//    }
//    
//    std::cout << "doSelect end" << std::endl;
//    
//    return 0;
//}

//int direct_forwarding(ssh_session session, ssh_channel guest_channel, const char * host, int port)
//{
//    ssh_channel forwarding_channel;
//    int rc;
//    std::string http_get = std::string("GET / HTTP/1.1\nHost: www.google.com\n\n");
//    int nbytes, nwritten;
//    forwarding_channel = ssh_channel_new(session);
//    if (forwarding_channel == NULL) {
//        return rc;
//    }
//    ssh_channel_set_blocking(forwarding_channel, 1);
//    rc = ssh_channel_open_forward(forwarding_channel,
//                                  "www.google.com", 80,
//                                  host, port);
//    if (rc != SSH_OK)
//    {
//        ssh_channel_free(forwarding_channel);
//        return rc;
//    }
//    std::cout << "coucou0" << std::endl;
//    nbytes = strlen(http_get.c_str());
//    nwritten = ssh_channel_write(forwarding_channel,
//                                 http_get.c_str(),
//                                 nbytes);
//    std::cout << "nbytes(" << nbytes << ")" << std::endl;
//    std::cout << "nwritten(" << nwritten << ")" << std::endl;
//    if (nbytes != nwritten)
//    {
//        std::cout << "coucou-1" << std::endl;
//        ssh_channel_free(forwarding_channel);
//        return SSH_ERROR;
//    }else{
//        std::cout << "coucou1" << std::endl;
//    }
//    
//    ssh_channel_free(forwarding_channel);
//    return SSH_OK;
//}

int direct_forwarding(ssh_session session, const char * host)
{
    ssh_channel forwarding_channel;
    int rc;
    std::string http_get = std::string("GET / HTTP/1.1\nHost: www.google.com\n\n");
    int nbytes, nwritten;
    forwarding_channel = ssh_channel_new(session);
    if (forwarding_channel == NULL) {
        return rc;
    }
    rc = ssh_channel_open_forward(forwarding_channel,
                                  "www.google.com", 80,
                                  "localhost", 5555);
    int opened = ssh_channel_is_open(forwarding_channel);
    if (opened == 1){
        std::cout << "channel opened" << std::endl;;
    }else{
        std::cout << "channel not opened" << std::endl;;
    }
    
    if (rc != SSH_OK)
    {
        ssh_channel_free(forwarding_channel);
        return rc;
    }
    nbytes = strlen(http_get.c_str());
    nwritten = ssh_channel_write(forwarding_channel,
                                 http_get.c_str(),
                                 nbytes);
    if (nbytes != nwritten)
    {
        ssh_channel_free(forwarding_channel);
        return SSH_ERROR;
    }

    
    ssh_channel_free(forwarding_channel);
    return SSH_OK;
}

//int direct_forwarding(ssh_session session, const char * host, int port)
//{
//    ssh_channel forwarding_channel;
//    //ssh_event event;
//    //short events;
//    int rc;
////    socket_t fd;
//    
//    forwarding_channel = ssh_channel_new(session);
//    if (forwarding_channel == NULL) {
//        return rc;
//    }
//    
//    ssh_channel_set_blocking(forwarding_channel, 1);
//    rc = ssh_channel_open_forward(forwarding_channel,
//                                  "192.168.1.230", 22,
//                                  host, 23);
//    
//    std::cout << "ssh_channel_open_forward status : " << rc << std::endl;
//    if (rc != SSH_OK)
//    {
//        ssh_channel_free(forwarding_channel);
//        return rc;
//    }else{
//        std::cout << "Did forward the connection" << std::endl;
//    }
//
//    
//    std::cout << "ssh_channel_is_open : " << ssh_channel_is_open(forwarding_channel) << std::endl;
//    
//    cb.userdata = &fd;
//    ssh_callbacks_init(&cb);
//    ssh_set_channel_callbacks(forwarding_channel, &cb);
//    
//    events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
//    
//    event = ssh_event_new();
//    if(event == NULL) {
//        printf("Couldn't get a event\n");
//        return -1;
//    }
//    if(ssh_event_add_fd(event, fd, events, copy_fd_to_chan, forwarding_channel) != SSH_OK) {
//        printf("Couldn't add an fd to the event\n");
//        return -1;
//    }
//    if(ssh_event_add_session(event, session) != SSH_OK) {
//        printf("Couldn't add the session to the event\n");
//        return -1;
//    }
    
//    do {
//        ssh_event_dopoll(event, 1000);
//    } while(!ssh_channel_is_closed(forwarding_channel));
//    
//    if (doSelect(session, forwarding_channel) < 0){
//        ssh_channel_free(forwarding_channel);
//        return -1;
//    }
    
//    ssh_channel_free(forwarding_channel);
//    return SSH_OK;
//}


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
    
    //web_server(_session);
    
    //direct_forwarding(_session, host.c_str(), port);
    direct_forwarding(_session, host.c_str());
    return 0;
}