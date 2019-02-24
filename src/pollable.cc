#include "pollable.h"

#include "glog/logging.h"
#include "net.h"

namespace hera {

Pollable::~Pollable() {
  if (events_ != 0) {
    EpollCtl(efd_, fd_, EPOLL_CTL_DEL, 0);
  }
  Close(fd_);
}

bool Pollable::EnableRead() {
  return AddEvent(EPOLLPRI | EPOLLIN);
}

bool Pollable::EnableWrite() {
  return AddEvent(EPOLLOUT);
}

bool Pollable::DisableWrite() {
  return DelEvent(EPOLLOUT);
}

bool Pollable::AddEvent(int events) {
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

bool Pollable::DelEvent(int events) {
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
