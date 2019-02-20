#pragma once

#include <arpa/inet.h>

#include <memory>

#include "handler.h"

namespace hera {

class Connection {
 public:
  Connection(int efd, int fd, sockaddr_in addr, HandlerPtr handler);
  virtual ~Connection();

  Connection(const Connection&) = delete;
  Connection(Connection&&) = delete;
  Connection& operator=(const Connection&) = delete;
  Connection& operator=(Connection&&) = delete;

  bool EnableRead();
  bool EnableWrite();
  bool DisableRead();
  bool DisableWrite();
  bool OnRead();
  bool OnWrite();
  ssize_t Read(void* buf, size_t count);
  ssize_t Write(const void* buf, size_t count);

  int fd() const { return fd_; }
  const char* addr_id() const { return addr_id_; }

 private:
  static const int kAddrIpLen = 16;
  static const int kAddrIdLen = 22; 

  bool AddEvent(int events);
  bool DelEvent(int events);

  int efd_;
  int fd_;
  int events_;
  sockaddr_in addr_;
  char addr_id_[kAddrIdLen];
  HandlerPtr handler_;
};

using ConnectionPtr = std::shared_ptr<Connection>;

}  // namespace hera
