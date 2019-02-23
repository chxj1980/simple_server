#include "server.h"

#include <signal.h>

#include "glog/logging.h"
#include "net.h"

namespace hera {

Server::~Server() {
  Close(lfd_);
  Close(efd_);
}

bool Server::Init() {
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    LOG(ERROR) << "ignore SIGPIPE error: " << strerror(errno);
    return false;
  }
  int needed_fds = max_connections_ + 1024;
  if (!SetRlimitNofile(needed_fds)) {
    return false;
  }
  efd_ = EpollCreate();
  if (efd_ == -1) {
    return false;
  }
  lfd_ = Listen(port_, backlog_);
  if (lfd_ == -1) {
    return false;
  }
  if (!EpollCtl(efd_, lfd_, EPOLL_CTL_ADD, EPOLLIN)) {
    return false;
  }
  LOG(INFO) << "listen on port: " << port_;
  return true;
}

void Server::Start() {
  // TODO(litao.sre): while running
  while (true) {
    if (!Poll()) {
      LOG(ERROR) << "poll error";
    }
  }
}

void Server::Stop() {
}

ConnectionPtr Server::AcceptConn() {
  sockaddr_in addr;
  int fd = Accept(lfd_, addr);
  if (fd == -1) {
    return nullptr;
  }
  if (connections_.size() >= max_connections_) {
    LOG(WARNING) << "exceed max connections: " << max_connections_;
    Close(fd);
    return nullptr;
  }
  HandlerPtr handler = CreateHandler();
  ConnectionPtr conn = std::make_shared<Connection>(efd_, fd, addr, handler);
  if (!conn->EnableRead()) {
    LOG(ERROR) << "conn enable read error";
    return nullptr;
  }
  return conn;
}

bool Server::Poll() {
  int nfds = EpollWait(efd_, events_.data(), events_.size());
  if (nfds == -1) {
    return false;
  }
  for (int i = 0; i < nfds; ++i) {
    const epoll_event& event = events_[i];
    if (event.data.fd == lfd_) {
      ConnectionPtr conn = AcceptConn();
      if (conn) {
        LOG(INFO) << "accept: " << conn->addr_id();
        connections_[conn->fd()] = conn;
      }
    } else {
      int fd = event.data.fd;
      int events = event.events;
      if (connections_.find(fd) == connections_.end()) {
        LOG(ERROR) << "epoll fd not found";
        Close(fd);
        continue;
      }
      ConnectionPtr conn = connections_[fd];
      if (events & (EPOLLHUP | EPOLLERR)) {
        LOG(ERROR) << "epoll error events: " << events;
        connections_.erase(fd);
      } else if (events & (EPOLLPRI | EPOLLIN)) {
        if (!conn->OnRead()) {
          LOG(INFO) << "close: " << conn->addr_id();
          connections_.erase(fd);
        }
      } else if (events & EPOLLOUT) {
        if (!conn->OnWrite()) {
          LOG(INFO) << "close: " << conn->addr_id();
          connections_.erase(fd);
        }
      }
    }
  }
  return true;
}

}  // namespace hera
