#include "server.hpp"

Server::Server(int p, const std::string& ap, const std::string& r, const std::string& c) :
  port(p), api_prefix(ap), root(r), config_path(c) {
  setup();
}

Server::Server(const Json::Value& config) {
}

Server::Server(const std::string fname) {
}

void Server::setup() {
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, port.c_str(), ev_handler);
  s_http_server_opts.document_root = root.c_str();
  mg_set_protocol_http_websocket(nc);
}

Server::~Server() {
  mg_mgr_free(&mgr);
}

void Server::poll(int delay) {
  mg_mgr_poll(&mgr, delay);
}

void Server::http_request() {
}

void Server::http_get() {
}

void Server::http_post() {
}

void Server::http_delete() {
}

void Server::http_response() {
}

void Server::websocket_create() {
}

void Server::websocket_frame() {
}

void Server::websocket_close() {
}

