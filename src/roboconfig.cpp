#include <iostream>
#include "server.hpp"
#include "config.hpp"

using namespace std;

bool is_prefix(string const& prefix, string const& str) {
    return equal(prefix.begin(), prefix.end(),
//            prefix.begin() + std::min(prefix.size(), str.size()),
            str.begin());
}

class RoboConfig : public Server {
public:
    RoboConfig(const std::string fname) : Server(fname), api_base("/api/config"), robot_config("config") {}

    void http_post(struct mg_connection *nc, struct http_message *hm) override;
    void http_get(struct mg_connection *nc, struct http_message *hm) override;
    void http_delete(struct mg_connection *nc, struct http_message *hm) override;

private:
    bool if_api(struct http_message *hm) { return if_api(asString(hm->uri)); }
    bool if_api(const string& uri) { return is_prefix(api_base, uri); }

    string api_base;
    Config robot_config;
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
            robot_config.remove(uri.substr(api_base.size()));
            cout << "delete " << uri << endl;
            send_http_response(nc);
        }
    } else {
        Server::http_delete(nc, hm);
    }
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
        server.poll();
    }

    return 0;
}

