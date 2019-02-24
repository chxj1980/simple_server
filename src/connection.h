#pragma once

#include <arpa/inet.h>

#include <memory>

#include "handler.h"
#include "pollable.h"

namespace hera {

class Connection : public Pollable {
 public:
  Connection(int efd, int fd, sockaddr_in addr, HandlerPtr handler);
  virtual ~Connection();

  bool OnRead() override;
  bool OnWrite() override;

  const char* addr_id() const { return addr_id_; }
  BufferPtr read_buf() const { return read_buf_; }
  BufferPtr write_buf() const { return write_buf_; }

 private:
  static const size_t kAddrIpLen;
  static const size_t kAddrIdLen;
  static const size_t kMinBufCap;
  static const size_t kBufExpandThreshold;

  sockaddr_in addr_;
  char* addr_id_;
  HandlerPtr handler_;
  BufferPtr read_buf_;
  BufferPtr write_buf_;
};

using ConnectionPtr = std::shared_ptr<Connection>;

}  // namespace hera
