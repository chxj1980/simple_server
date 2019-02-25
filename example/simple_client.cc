#include "glog/logging.h"
#include "net.h"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  if (argc != 3) {
    LOG(ERROR) << "usage: ./simple_client <host> <port>";
    exit(EXIT_FAILURE);
  }
  char* host = argv[1];
  int port = atoi(argv[2]);
  int fd = hera::Connect(host, port);
  if (fd == -1) {
    exit(EXIT_FAILURE);
  }
  LOG(INFO) << "connect " << host << ":" << port;
  char req[] = "TIME";
  write(fd, req, sizeof(req));
  LOG(INFO) << "send req: " << req;
  uint64_t resp;
  read(fd, &resp, sizeof(resp));
  LOG(INFO) << "recv resp: " << resp;
  close(fd);
  LOG(INFO) << "close connection";
  google::ShutdownGoogleLogging();
  return EXIT_SUCCESS;
}
