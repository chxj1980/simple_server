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
  bool DisableWrite();

  bool OnRead();
  bool OnWrite();

  int fd() const { return fd_; }
  const char* addr_id() const { return addr_id_; }
  BufferPtr read_buf() const { return read_buf_; }
  BufferPtr write_buf() const { return write_buf_; }

 private:
  static const size_t kAddrIpLen;
  static const size_t kAddrIdLen;
  static const size_t kMinBufCap;
  static const size_t kBufExpandThreshold;

  bool AddEvent(int events);
  bool DelEvent(int events);

  int efd_;
  int fd_;
  int events_;
  sockaddr_in addr_;
  char* addr_id_;
  HandlerPtr handler_;
  BufferPtr read_buf_;
  BufferPtr write_buf_;
};

using ConnectionPtr = std::shared_ptr<Connection>;

}  // namespace hera
