#include <chrono>

#include "glog/logging.h"
#include "server.h"

class MyHandler : public hera::Handler {
 public:
  hera::Handler::ReqStatus OnRequest(hera::Connection* conn) override;
  bool OnComplete(bool succ) override;
};

hera::Handler::ReqStatus MyHandler::OnRequest(hera::Connection* conn) {
  if (conn->read_buf()->limit() < 5) {
    return hera::Handler::kReqNotReady;
  }
  const char* req = conn->read_buf()->head();
  if (strcmp(req, "TIME") != 0) {
    LOG(ERROR) << "invalid req: " << req;
    return hera::Handler::kReqInvalid;
  }
  LOG(INFO) << "recv req: " << req;

  std::time_t resp = std::time(nullptr);
  conn->write_buf()->Write(&resp, sizeof(resp));
  conn->EnableWrite();
  LOG(INFO) << "send resp: " << resp;

  return hera::Handler::kReqReady;
}

bool MyHandler::OnComplete(bool succ) {
  if (succ) {
    LOG(INFO) << "resp send succ";
  } else {
    LOG(INFO) << "resp send failed";
  }
  return true;
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
