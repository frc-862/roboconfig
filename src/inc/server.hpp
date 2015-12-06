#ifndef SERVER_HPP__
#define SERVER_HPP__

#include <string>
#include "json/json.h"
#include "mongoose.h"

class Server {
public:
  Server(const std::string& p, const std::string& ap, const std::string& r, const std::string& c);
  Server(const Json::Value& config);
  Server(const std::string fname);

  virtual ~Server();

  virtual void idle(struct mg_connection*) {}
  virtual void poll(int sleep = 100);
  virtual void http_request(struct mg_connection *nc, int ev, struct http_message *hm);
  virtual void http_get(struct mg_connection *nc, struct http_message *hm);
  virtual void http_post(struct mg_connection *nc, struct http_message *hm);
  virtual void http_delete(struct mg_connection *nc, struct http_message *hm);
 
  virtual void websocket_request(struct mg_connection *nc, struct http_message *hm);
  virtual void websocket_create(struct mg_connection *nc);
  virtual void websocket_frame(struct mg_connection *nc, struct websocket_message *wm);
  virtual void websocket_close(struct mg_connection *nc);
  
  static bool is_equal(const struct mg_str *s1, const struct mg_str *s2) {
    return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
  }

  static const struct mg_str get_method;
  static const struct mg_str post_method;
  static const struct mg_str delele_method;

  void send_http_response(struct mg_connection *nc, const std::string& body="", int code=200, const std::string& reason="OK");
  void send_http_error(struct mg_connection *nc, int code, const std::string& reason="");

  void send_http_json_response(struct mg_connection *nc, const Json::Value& response, int code=200, const std::string& reason="OK");

protected:
  struct mg_mgr mgr;
  struct mg_serve_http_opts server_opts;
  virtual void event_handler(struct mg_connection *nc, int ev, void *ev_data);

  virtual void setup();
  virtual void setup(const Json::Value& config);

  std::string port;
  std::string api_prefix;
  std::string root;
  std::string config_path; 

private:
  static void raw_event_handler(struct mg_connection *nc, int ev, void *ev_data);
};

#endif

