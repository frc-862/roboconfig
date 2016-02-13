#include <iostream>
#include "server.h"
#include "config.h"
#include "ws_file_tail.h"
#include "traj.h"

using namespace std;

bool is_prefix(string const& prefix, string const& str) {
    return equal(prefix.begin(), prefix.end(), str.begin());
}

class RoboConfig : public Server {
public:
    RoboConfig(const std::string fname) : Server(fname), 
            api_root(get_config("api_root", "/api")), 
            api_activate(api_root + "/activate"), api_config(api_root + "/config"),
	  	  	  robot_config(get_config("config_root", "config"), get_config("robot_config", "/home/lvuser/sirius.conf")), 
            log_path(get_config("log_path", "logs")) {}

    void http_post(struct mg_connection *nc, struct http_message *hm) override;
    void http_get(struct mg_connection *nc, struct http_message *hm) override;
    void http_delete(struct mg_connection *nc, struct http_message *hm) override;

    virtual void websocket_request(struct mg_connection *nc, struct http_message *hm) override;
    virtual void websocket_create(struct mg_connection *nc) override;
    virtual void websocket_frame(struct mg_connection *nc, struct websocket_message *wm) override;
    virtual void websocket_close(struct mg_connection *nc) override;

    virtual void idle(mg_connection *connection) override;

private:
    bool if_generate_route(const string& uri) { return is_prefix("/route", uri); }
    bool if_generate_route(struct http_message *hm) { return if_generate_route(asString(hm->uri)); }
    bool if_api_config(struct http_message *hm) { return if_api_config(asString(hm->uri)); }
    bool if_api_config(const string& uri) { return is_prefix(api_config, uri); }
    bool if_activate(struct http_message *hm) { return if_activate(asString(hm->uri)); }
    bool if_activate(const string& uri) { return is_prefix(api_activate, uri); }

    string api_root;
    string api_activate;
    string api_config;
    Config robot_config;
    string log_path;
};

void RoboConfig::http_get(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);

    if (if_api_config(uri)) {
        if (uri == api_config) {
            // this is an index request, return all our config files
            send_http_json_response(nc, robot_config.get());
        } else {
            // find the correct config file and return it
            send_http_json_response(nc, robot_config.get(uri.substr(api_config.size())));
        }
    } else if (if_activate(uri)) {
      send_http_json_response(nc, robot_config.activated());

    } else {
        Server::http_get(nc, hm);
    }
}

void RoboConfig::http_post(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);

    if (if_generate_route(uri)) {
      Json::Value req = json_body(hm);
      if (req != Json::nullValue)
      {
        send_http_json_response(nc, generate_path(req));
      } else {
        send_http_error(nc, 400, "Invalid JSON Body");
      }
    } else if (if_api_config(uri)) {
        if (uri == api_config) {
            send_http_error(nc, 405, "Cannot set root");
        } else {
            if (robot_config.set(uri.substr(api_config.size()), asString(hm->body)))
                send_http_response(nc);
            else
                send_http_error(nc, 400, "Invalid JSON Body");
        }
    } else if (if_activate(uri)) {
        if (uri == api_activate) {
            // index request, is not supported
        	send_http_error(nc, 400, "You cannot activate a missing configuration");
        } else {
        	if (robot_config.activate(uri.substr(api_activate.size())))
        		send_http_response(nc);
        	else
        		send_http_error(nc, 404, "Could not activate");
        }
    } else {
        Server::http_post(nc, hm);
    }
}

void RoboConfig::http_delete(struct mg_connection *nc, struct http_message *hm) {
    string uri = asString(hm->uri);

    if (if_api_config(uri)) {
        if (uri == api_config) {
            send_http_error(nc, 405, "Cannot delete root");
        } else {
            if (robot_config.remove(uri.substr(api_config.size())))
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
    WSFileTail* wsft = new WSFileTail(log_path + uri.substr(5));

    if (!wsft->good()) {
        delete wsft;
        Server::websocket_request(nc, hm);
    } else {
        nc->user_data = wsft;
    }
}

void RoboConfig::websocket_create(struct mg_connection *nc) {
    WSFileTail* wsft = (WSFileTail *) nc->user_data;
    string lines = wsft->lines().toStyledString();
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, lines.c_str(), lines.size());
}

void RoboConfig::websocket_frame(struct mg_connection *nc, struct websocket_message *wm) {
    Server::websocket_frame(nc, wm);
}

void RoboConfig::websocket_close(struct mg_connection *nc) {
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
