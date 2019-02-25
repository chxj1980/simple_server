#include "connection.h"

#include "glog/logging.h"
#include "net.h"

namespace hera {

const size_t Connection::kAddrIpLen = 16;
const size_t Connection::kAddrIdLen = 22;
const size_t Connection::kMinBufCap = 1024;
const size_t Connection::kBufExpandThreshold = 128;

Connection::Connection(int efd, int fd, sockaddr_in addr, HandlerPtr handler) :
    efd_(efd), fd_(fd), events_(0), addr_(addr), handler_(handler),
    read_buf_(std::make_shared<Buffer>(kMinBufCap)),
    write_buf_(std::make_shared<Buffer>(kMinBufCap)) {
  uint16_t port = ntohs(addr_.sin_port);
  char ip[kAddrIpLen];
  inet_ntop(AF_INET, &(addr.sin_addr), ip, sizeof(ip));
  addr_id_ = new char[kAddrIdLen];
  snprintf(addr_id_, kAddrIdLen, "%s:%d", ip, port);
}

Connection::~Connection() {
  if (events_ != 0) {
    EpollCtl(efd_, fd_, EPOLL_CTL_DEL, 0);
  }
  Close(fd_);
  delete[] addr_id_;
}

bool Connection::EnableRead() {
  return AddEvent(EPOLLPRI | EPOLLIN);
}

bool Connection::EnableWrite() {
  return AddEvent(EPOLLOUT);
}

bool Connection::DisableWrite() {
  return DelEvent(EPOLLOUT);
}

bool Connection::OnRead() {
  do {
    if (read_buf_->remaining() < kBufExpandThreshold) {
      read_buf_->Expand(read_buf_->capacity());
    }
    ssize_t num = read(fd_, read_buf_->tail(), read_buf_->remaining());
    if (num == 0) {
      LOG(INFO) << "peer closed: " << addr_id();
      return false;
    } else if (num < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return true;
      }
      LOG(ERROR) << "read error: " << strerror(errno);
      return false;
    } else {
      read_buf_->incr_limit(num);
      switch (handler_->OnRequest(this)) {
        case Handler::kReqReady:
          return true;
        case Handler::kReqNotReady:
          break;
        default:
          return false;
      }
    }
  } while (true);
}

bool Connection::OnWrite() {
  do {
    int num = write(fd_, write_buf_->head(), write_buf_->size());
    if (num == 0) {
      return true;
    } else if (num < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return true;
      }
      LOG(ERROR) << "write error: " << strerror(errno);
      DisableWrite();
      return handler_->OnComplete(false);
    } else {
      write_buf_ -> incr_position(num);
      if (write_buf_->empty()) {
        DisableWrite();
        return handler_->OnComplete(true);
      }
    }
  } while (true);
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
