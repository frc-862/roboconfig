#include "server.hpp"
#include "config.hpp"

class RoboConfig : public Server {
public:
  RoboConfig(const std::string fname) : Server(fname) {}


};

static sig_atomic_t s_signal_received = 0;

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);  // Reinstantiate signal handler
  s_signal_received = sig_num;
}

int main(void) {
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  RoboConfig server("roboconfig.json");
  printf("server at %p\n", &server);

  while (s_signal_received == 0) {
    server.poll();
  }

  return 0;
}

