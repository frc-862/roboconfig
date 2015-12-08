#include <iostream>
#include "server.h"
#include "config.h"
#include "ws_file_tail.h"

using namespace std;

bool is_prefix(string const& prefix, string const& str) {
    return equal(prefix.begin(), prefix.end(),
//            prefix.begin() + std::min(prefix.size(), str.size()),
            str.begin());
}

class RoboConfig : public Server {
public:
    RoboConfig(const std::string fname) : Server(fname), api_base("/api/config"), robot_config("config"), log_path("logs") {}

    void http_post(struct mg_connection *nc, struct http_message *hm) override;
    void http_get(struct mg_connection *nc, struct http_message *hm) override;
    void http_delete(struct mg_connection *nc, struct http_message *hm) override;


    virtual void websocket_request(struct mg_connection *nc, struct http_message *hm) override;
    virtual void websocket_create(struct mg_connection *nc) override;
    virtual void websocket_frame(struct mg_connection *nc, struct websocket_message *wm) override;
    virtual void websocket_close(struct mg_connection *nc) override;

    virtual void idle(mg_connection *connection) override;

private:
    bool if_api(struct http_message *hm) { return if_api(asString(hm->uri)); }
    bool if_api(const string& uri) { return is_prefix(api_base, uri); }

    string api_base;
    Config robot_config;
    string log_path;
};

void RoboConfig::http_get(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);

    if (if_api(uri)) {
        if (uri == api_base) {
            // this is an index request, return all our config files
            send_http_json_response(nc, robot_config.get());
        } else {
            // find the correct config file and return it
            send_http_json_response(nc, robot_config.get(uri.substr(api_base.size())));
        }
    } else {
        Server::http_get(nc, hm);
    }
}

void RoboConfig::http_post(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);

    if (if_api(uri)) {
        if (uri == api_base) {
            send_http_error(nc, 405, "Cannot set root");
        } else {
            if (robot_config.set(uri.substr(api_base.size()), asString(hm->body)))
                send_http_response(nc);
            else
                send_http_error(nc, 400, "Invalid JSON Body");
        }
    } else {
        Server::http_post(nc, hm);
    }
}

void RoboConfig::http_delete(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);

    if (if_api(uri)) {
        if (uri == api_base) {
            send_http_error(nc, 405, "Cannot delete root");
        } else {
            if (robot_config.remove(uri.substr(api_base.size())))
                send_http_response(nc);
            else
                send_http_error(nc, 404, "Unable to delete");
        }
    } else {
        Server::http_delete(nc, hm);
    }
}

void RoboConfig::websocket_request(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);
    // Expect ws://tail/fname
    WSFileTail* wsft = new WSFileTail(log_path + "/" + uri.substr(5));

    if (!wsft->good()) {
        delete wsft;
        Server::websocket_request(nc, hm);
    } else {
        nc->user_data = wsft;
    }
}

void RoboConfig::websocket_create(struct mg_connection *nc) {
    cout << "websocket create" << endl;
    WSFileTail* wsft = (WSFileTail *) nc->user_data;
    string lines = wsft->lines().toStyledString();
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, lines.c_str(), lines.size());
}

void RoboConfig::websocket_frame(struct mg_connection *nc, struct websocket_message *wm) {
    cout << "websocket frame" << endl;
    Server::websocket_frame(nc, wm);
}

void RoboConfig::websocket_close(struct mg_connection *nc) {
    cout << "web socket disconnect" << endl;
    delete ((WSFileTail *) nc->user_data);
    nc->user_data = nullptr;
    Server::websocket_close(nc);
}

static sig_atomic_t s_signal_received = 0;

static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);  // Reinstantiate signal handler
    s_signal_received = sig_num;
}

int main(void) {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    RoboConfig server("roboconfig.json");
    cout << "RoboConfig listening on port " << server.get_port() << endl;

    while (s_signal_received == 0) {
        server.poll(1000);
    }

    return 0;
}

void RoboConfig::idle(mg_connection *nc) {
    for (auto c = mg_next(nc->mgr, nullptr); c != nullptr; c = mg_next(nc->mgr, c)) {
        if (nc->flags & MG_F_IS_WEBSOCKET && c->user_data != nullptr) {
            WSFileTail *wsft = (WSFileTail *) c->user_data;
            Json::Value lines = wsft->lines();
            if (lines != Json::nullValue) {
                string result = lines.toStyledString();
                mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, result.c_str(), result.size());
            }
        }
    }
}
