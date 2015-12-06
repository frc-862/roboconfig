#ifndef SERVER_HPP__
#define SERVER_HPP__

#include "mongoose.h"

class Server {
public:
  Server(int p=8080, const std::string& ap="/api", const std::string& r="public", const std::string& c="config");
  Server(const Json::Value& config);
  Server(const std::string fname);

  virtual ~Server();

  virtual void poll(int sleep = 100);
  virtual void http_request();
  virtual void http_get();
  virtual void http_post();
  virtual void http_delete();
  virtual void http_response();
 
  virtual void websocket_create();
  virtual void websocket_frame();
  virtual void websocket_close();
  
protected:
  struct mg_mgr mgr;

  virtual void setup();

  int port;
  std::string api_prefix;
  std::string root;
  std::string config_path; 
};

#endif

