//
//  SPTAclController.cpp
//  SpatchServer
//
//  Created by Pichon Nathan on 04/02/2016.
//  Copyright 2016 Pichon Nathan. All rights reserved.


#include "SPTAclController.hpp"

SPTAclController::SPTAclController(std::string user, std::string password):
  _user(user),
  _password(password),
  _servers(NULL) {
  _fd = fopen("acl/acl", "rb");
  char buffer[65536];
  rapidjson::FileReadStream jsonFile(_fd, buffer, sizeof(buffer));
  _file.ParseStream(jsonFile);

  
}

SPTAclController::~SPTAclController() {
  fclose(_fd);
}

std::list<std::pair<std::string, std::string>> SPTAclController::getServers() {
  return _servers;
}

