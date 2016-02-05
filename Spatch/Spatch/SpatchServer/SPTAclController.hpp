#ifndef SPTAclController_hpp
#define SPTAclController_hpp

#include "rapidjson/document.h>"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <fstream>
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include <cstdio>

class SPTAclController{

public:
  SPTAclController(std::string username, std::string password);
  ~SPTAclController();
  int getServers();

private:
  std::list _servers;
  rapidjson::Document _file;
  FILE* _fd;
  std::string _user;
  std::string _password;

};

#endif /* SPTAclController_hpp  */
