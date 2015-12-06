#include<fstream>
#include<map>
#include<string>
#include<mutex>

#include "config.hpp"

#include "tinydir.h"
#include "json/json.h"
#include "mongoose.h"

static std::mutex robot_mutex;
static std::map<std::string, Json::Value> robot_config_data;

static sig_atomic_t s_signal_received = 0;
static std::string s_http_port("8000");
static std::string s_document_root("public");
static struct mg_serve_http_opts s_http_server_opts;
static const struct mg_str s_get_method = MG_STR("GET");
static const struct mg_str s_post_method = MG_STR("POST");
static const struct mg_str s_delele_method = MG_STR("DELETE");
static std::string s_api_prefix("/api/v1/config");
static struct mg_str api_prefix = { s_api_prefix.c_str(), s_api_prefix.size() };
static std::string s_config_directory("config");

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);  // Reinstantiate signal handler
  s_signal_received = sig_num;
}

static int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}

static void broadcast(struct mg_connection *nc, const char *msg, size_t len) {
  struct mg_connection *c;
  char buf[500];

  snprintf(buf, sizeof(buf), "%p %.*s", nc, (int) len, msg);
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
  return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static int is_equal(const struct mg_str *s1, const struct mg_str *s2) {
  return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}

void return_json(struct mg_connection *nc, const Json::Value& value) {
  std::string buf = value.toStyledString();

  mg_printf(nc, "HTTP/1.1 200 OK\r\n"
      "Content-Length: %d\r\n"
      "Content-Type: application/json\r\n\r\n"
      "%s",
      (int) buf.size(), buf.c_str());

  //mg_send_http_chunk(nc, "", 0);
  printf("We are done\n");
}

Json::Value read_config(const std::string& name) {
  Json::Value result;
  std::lock_guard<std::mutex> lock(robot_mutex);

  auto value = robot_config_data.find(name);
  if (value == robot_config_data.end()) {
    // read (if it exists) the config file
    std::ifstream stream(s_config_directory + "/" + name + ".json", std::ifstream::binary);
    if (stream.good()) {
      stream >> result;
      robot_config_data[name] = result;
    }
  } else {
    // return what we have
    result = value->second;
  }

  return result;
}

void config_get(struct mg_connection *nc, const struct mg_str *key) {
  Json::Value result;

  // check is this an index request (return all config files)
  if ((key->len == 1) && (*key->p == '/'))
  {
    result = Json::arrayValue;
    tinydir_dir dir;
    tinydir_open(&dir, s_config_directory.c_str());

    for (; dir.has_next; tinydir_next(&dir))
    {
      tinydir_file file;
      tinydir_readfile(&dir, &file);

      if (!file.is_dir)
        result.append(file.name);
    }
    tinydir_close(&dir);
  } else {
    // or a get of a particular config file
    result = read_config(std::string(key->p + 1, key->len - 1));
  }

  return_json(nc, result);
  printf("exit config_get\n");
}

static void config_write(struct mg_connection *nc, const struct http_message *hm, const struct mg_str *key) {

  Json::Value value;
  Json::Reader reader;
  if (reader.parse(hm->body.p, hm->body.p + hm->body.len, value))
  {
    std::string name(key->p + 1, key->len - 1);
    printf("We have good json: %s\n", value.toStyledString().c_str());

    std::lock_guard<std::mutex> lock(robot_mutex);
    robot_config_data[name] = value;
    std::ofstream of(s_config_directory + "/" + name + ".json");
    of << value.toStyledString();
  } else {
    printf("Failed to parse json :-(\n");
    printf("Body: %.*s\n", (int) hm->body.len, hm->body.p);
  }

  mg_printf(nc, "%s",
      "HTTP/1.0 200 OK\r\n"
      "Content-Length: 0\r\n\r\n");
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;
  struct websocket_message *wm = (struct websocket_message *) ev_data;
  struct mg_str key;

  switch (ev) {
    case MG_EV_HTTP_REQUEST:
      if (has_prefix(&hm->uri, &api_prefix)) {
        key.p = hm->uri.p + api_prefix.len;
        key.len = hm->uri.len - api_prefix.len;
        if (is_equal(&hm->method, &s_get_method)) {
          config_get(nc, &key);
        } else if (is_equal(&hm->method, &s_post_method)) {
          config_write(nc, hm, &key);
        } else if (is_equal(&hm->method, &s_delele_method)) {
          //db_op(nc, hm, &key, 3);
        } else {
          mg_printf(nc, "%s",
                    "HTTP/1.0 501 Not Implemented\r\n"
                    "Content-Length: 0\r\n\r\n");
        }
      } else {
        mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
      }
      break;

    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
      /* New websocket connection. Tell everybody. */
      broadcast(nc, "joined", 6);
      break;

    case MG_EV_WEBSOCKET_FRAME:
      /* New websocket message. Tell everybody. */
      broadcast(nc, (char *) wm->data, wm->size);
      break;

    case MG_EV_CLOSE:
      /* Disconnect. Tell everybody. */
      if (is_websocket(nc)) {
        broadcast(nc, "left", 4);
      }
      break;

    case MG_EV_ACCEPT:
      //if (nc->user_data == nullptr)
      //{
        //nc->user_data = (void*) (new std::string("foo"))->c_str();
      //}
      break;

    case MG_EV_POLL:
      if (is_websocket(nc)) {
        mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, "tick\n", 5);
      }
      break;

    default:
      printf("event %d\n", ev);
      break;
  }
}

static void setup_server() {
  Json::Value server_config;
  std::ifstream stream("roboconfig.json", std::ifstream::binary);
  stream >> server_config;

  s_http_port = server_config.get("port", "8000").asString();

  s_api_prefix = server_config.get("api_prefix", "/api/v1/config").asString();
  api_prefix.p = s_api_prefix.c_str();
  api_prefix.len = s_api_prefix.size();

  s_document_root = server_config.get("document_root", "public").asString();
  s_config_directory = server_config.get("config_directory", "config").asString();
  
  printf("Config: >%s<\n", server_config.toStyledString().c_str());
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  setup_server();

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, s_http_port.c_str(), ev_handler);
  s_http_server_opts.document_root = s_document_root.c_str();
  mg_set_protocol_http_websocket(nc);

  printf("Started on port %s\n", s_http_port.c_str());
  printf("Using document root %s\n", s_document_root.c_str());
  printf("Config api present at %s\n", s_api_prefix.c_str());
  
  while (s_signal_received == 0) {
    mg_mgr_poll(&mgr, 200);
  }
  mg_mgr_free(&mgr);

  return 0;
}

