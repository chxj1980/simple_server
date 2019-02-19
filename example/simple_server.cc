#include <chrono>

#include "glog/logging.h"
#include "handler.h"
#include "server.h"

class MyHandler : public hera::Handler {
 public:
  virtual bool OnRead(hera::Connection* conn);
  virtual bool OnWrite(hera::Connection* conn);
};

bool MyHandler::OnRead(hera::Connection* conn) {
  char req[5];
  ssize_t n = read(conn->fd(), req, sizeof(req));
  if (n == 0) {
    LOG(INFO) << "peer closed";
    return false;
  }
  if (strcmp(req, "TIME") == 0) {
    LOG(INFO) << "recv req: " << req;
    conn->EnableWrite();
    return true;
  } else {
    LOG(ERROR) << "invalid req: " << req;
    return false;
  }
}

class MyServer : public hera::Server {
 public:
  MyServer(uint16_t port, int backlog, size_t max_connections) :
      Server(port, backlog, max_connections) {}

 protected:
  virtual hera::HandlerPtr CreateHandler() {
    return std::make_shared<MyHandler>();
  }
};

bool MyHandler::OnWrite(hera::Connection* conn) {
  std::time_t resp = std::time(nullptr);
  write(conn->fd(), &resp, sizeof(resp));
  conn->DisableWrite();
  LOG(INFO) << "send resp: " << resp;
  return true;
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  if (argc != 2) {
    LOG(ERROR) << "usage: ./simple_server <port>";
    exit(EXIT_FAILURE);
  }
  uint16_t port = atoi(argv[1]);
  int backlog = 128;
  size_t max_connections = 1024;
  MyServer server(port, backlog, max_connections);
  if (!server.Init()) {
    LOG(ERROR) << "server init error";
    exit(EXIT_FAILURE);
  }
  // TODO(litao.sre): signal handle
  server.Start();
  google::ShutdownGoogleLogging();
  return 0;
}
