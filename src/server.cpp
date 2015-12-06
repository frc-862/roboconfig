#include <fstream>
#include "server.hpp"

const struct mg_str Server::get_method = MG_STR("GET");
const struct mg_str Server::post_method = MG_STR("POST");
const struct mg_str Server::delele_method = MG_STR("DELETE");

Server::Server(const std::string& p, const std::string& ap, const std::string& r, const std::string& c) :
    port(p), api_prefix(ap), root(r), config_path(c) {
    memset(&server_opts, 0, sizeof(server_opts));
    setup();
}

Server::Server(const Json::Value& config) {
    memset(&server_opts, 0, sizeof(server_opts));
    setup(config);
}

Server::Server(const std::string fname) {
    memset(&server_opts, 0, sizeof(server_opts));
    Json::Value server_config;
    std::ifstream stream(fname, std::ifstream::binary);
    stream >> server_config;

    setup(server_config);
}

void Server::setup() {
    struct mg_connection *nc;

    mg_mgr_init(&mgr, this);

    nc = mg_bind(&mgr, port.c_str(), raw_event_handler);
    server_opts.document_root = root.c_str();
    mg_set_protocol_http_websocket(nc);
}

void Server::setup(const Json::Value& config) {
    port = config.get("port", "8080").asString();
    api_prefix = config.get("api_prefix", "/api").asString();
    root = config.get("document_root", "public").asString();
    config_path = config.get("config_directory", "config").asString();

    setup();
}

Server::~Server() {
    mg_mgr_free(&mgr);
}

void Server::event_handler(struct mg_connection *nc, int ev, void *ev_data) {
    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        http_request(nc, ev, (struct http_message*) ev_data);
        break;

    case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
        websocket_request(nc, (struct http_message*) ev_data);
        break;

    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
        websocket_create(nc);
        break;

    case MG_EV_WEBSOCKET_FRAME:
        websocket_frame(nc, (struct websocket_message*) ev_data);
        break;

    case MG_EV_CLOSE:
        websocket_close(nc);
        break;

    case MG_EV_POLL:
        idle(nc);
        break;
    }
}

void Server::raw_event_handler(struct mg_connection *nc, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_REQUEST)
    {
        struct http_message *hm = (struct http_message*) ev_data;
        printf("raw request method >%.*s<\n", (int) hm->method.len, hm->method.p);
        printf("raw request proto >%.*s<\n", (int) hm->proto.len, hm->proto.p);
        printf("raw request uri >%.*s<\n", (int) hm->uri.len, hm->uri.p);
        printf("raw request >%s<\n", hm->message.p);
    }

    Server* me = (Server *) nc->mgr->user_data;
    me->event_handler(nc, ev, ev_data);
}

void Server::poll(int delay) {
    mg_mgr_poll(&mgr, delay);
}

void Server::http_request(struct mg_connection *nc, int, struct http_message *hm) {
    printf("http request >%.*s<\n", (int) hm->message.len, hm->message.p);
    if (is_equal(&hm->method, &get_method)) {
        http_get(nc, hm);

    } else if (is_equal(&hm->method, &post_method)) {
        http_post(nc, hm);

    } else if (is_equal(&hm->method, &delele_method)) {
        http_delete(nc, hm);

    }
}

void Server::http_get(struct mg_connection *nc, struct http_message *hm) {
    printf("Here we go, we have a GET\n");
    printf("Serving >%.*s< out of >%s<\n", (int) hm->uri.len, hm->uri.p, server_opts.document_root);
    printf("Message >%.*s<\n", (int) hm->message.len, hm->message.p);
    mg_serve_http(nc, hm, server_opts);
}

void Server::http_post(struct mg_connection *, struct http_message *) {
}

void Server::http_delete(struct mg_connection *, struct http_message *) {
}

void Server::websocket_request(struct mg_connection *, struct http_message *) {
}

void Server::websocket_create(struct mg_connection *) {
}

void Server::websocket_frame(struct mg_connection *, struct websocket_message *) {
}

void Server::websocket_close(struct mg_connection *) {
}

void Server::send_http_response(struct mg_connection *nc, const std::string& body, int code, const std::string& reason) {
    mg_printf(nc, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s", code, reason.c_str(), (int) body.size(), body.c_str());
}

void Server::send_http_error(struct mg_connection *nc, int code, const std::string& reason) {
    send_http_response(nc, "", code, reason);
}

void Server::send_http_json_response(struct mg_connection *nc, const Json::Value& response, int code, const std::string& reason) {
    std::string buf = response.toStyledString();

    mg_printf(nc, "HTTP/1.1 %d %s\r\n"
              "Content-Length: %d\r\n"
              "Content-Type: application/json\r\n\r\n"
              "%s",
              code, reason.c_str(),
              (int) buf.size(), buf.c_str());
}

