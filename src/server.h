#pragma once

#include <sys/epoll.h>

#include <atomic>
#include <vector>
#include <unordered_map>

#include "connection.h"

namespace hera {

class Server {
 public:
  Server(uint16_t port, int backlog, size_t max_connections) :
      interrupted_(false), port_(port), backlog_(backlog),
      max_connections_(max_connections), lfd_(-1), efd_(-1), ifd_(-1),
      events_(max_connections_) {}
  virtual ~Server();

  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(const Server&) = delete;
  Server& operator=(Server&&) = delete;

  bool Run();
  void Interrupt();
 
 protected:
  virtual HandlerPtr CreateHandler() = 0;

 private:
  enum PollStatus {
    kPollContinue = 0,
    kPollInterrupt = 1,
    kPollError = 2
  };

  ConnectionPtr AcceptConn();
  PollStatus Poll();

  std::atomic_bool interrupted_;
  uint16_t port_;
  int backlog_;
  size_t max_connections_;
  int lfd_;
  int efd_;
  int ifd_;
  std::vector<epoll_event> events_;
  std::unordered_map<int, ConnectionPtr> connections_;
};

}  // namespace hera
