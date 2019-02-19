#include "connection.h"

#include <chrono>

#include "glog/logging.h"
#include "net.h"

namespace hera {

Connection::Connection(int efd, int fd, sockaddr_in addr, HandlerPtr handler) :
    efd_(efd), fd_(fd), events_(0), addr_(addr), handler_(handler)  {
  uint16_t port = ntohs(addr_.sin_port);
  char ip[kAddrIpLen];
  inet_ntop(AF_INET, &(addr.sin_addr), ip, sizeof(ip));
  snprintf(addr_id_, sizeof(addr_id_), "%s:%d", ip, port);
}

Connection::~Connection() {
  if (events_ != 0) {
    EpollCtl(efd_, fd_, EPOLL_CTL_DEL, 0);
  }
  Close(fd_);
}

bool Connection::EnableRead() {
  return AddEvent(EPOLLPRI | EPOLLIN);
}


bool Connection::EnableWrite() {
  return AddEvent(EPOLLOUT | EPOLLET);
}

bool Connection::DisableRead() {
  return DelEvent(EPOLLPRI | EPOLLIN);
}

bool Connection::DisableWrite() {
  return DelEvent(EPOLLOUT);
}

bool Connection::OnRead() {
  return handler_->OnRead(this);
}

bool Connection::OnWrite() {
  return handler_->OnWrite(this);
}

bool Connection::AddEvent(int events) {
  int new_events = events_ | events;
  if (new_events != events_) {
    int op = events_ ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    if (!EpollCtl(efd_, fd_, op, new_events)) {
      return false;
    }
    events_ = new_events;
  }
  return true;
}

bool Connection::DelEvent(int events) {
  int new_events = events_ & ~events;
  if (new_events != events_) {
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    if (!EpollCtl(efd_, fd_, op, new_events)) {
      return false;
    }
    events_ = new_events;
  }
  return true;
}

}  // namespace hera
