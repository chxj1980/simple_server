#pragma once

namespace hera {

class Pollable {
 public:
  Pollable(int efd, int fd) : efd_(efd), fd_(fd), events_(0) {}
  virtual ~Pollable();

  Pollable(const Pollable&) = delete;
  Pollable(Pollable&&) = delete;
  Pollable& operator=(const Pollable&) = delete;
  Pollable& operator=(Pollable&&) = delete;

  bool EnableRead();
  bool EnableWrite();
  bool DisableWrite();

  virtual bool OnRead() = 0;
  virtual bool OnWrite() = 0;

  int fd() const { return fd_; }

 private:
  bool AddEvent(int events);
  bool DelEvent(int events);

  int efd_;
  int fd_;
  int events_;
};

}  // namespace hera
