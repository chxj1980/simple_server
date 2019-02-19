#pragma once

#include <sys/epoll.h>

#include <vector>
#include <unordered_map>

#include "connection.h"

namespace hera {

class Server {
 public:
  Server(uint16_t port, int backlog, size_t max_connections) :
      port_(port), backlog_(backlog), max_connections_(max_connections),
      lfd_(-1), efd_(-1), events_(max_connections_) {}
  virtual ~Server();

  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(const Server&) = delete;
  Server& operator=(Server&&) = delete;

  bool Init();
  void Start();
  void Stop();
 
 protected:
  virtual HandlerPtr CreateHandler() = 0;

 private:
  ConnectionPtr AcceptConn();
  bool Poll();

  uint16_t port_;
  int backlog_;
  size_t max_connections_;
  int lfd_;
  int efd_;
  std::vector<epoll_event> events_;
  std::unordered_map<int, ConnectionPtr> connections_;
};

}  // namespace hera
