#include "net.h"

#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/timerfd.h>

#include "glog/logging.h"

namespace hera {

bool SetRlimitNofile(int fds) {
  rlim_t total_fds = static_cast<rlim_t>(fds);
  rlimit rlim;
  if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) {
    LOG(ERROR) << "getrlimit error: " << strerror(errno);
    return false;
  }
  if (rlim.rlim_max < total_fds) {
      LOG(ERROR) << "fd hard limit less than needed: " << total_fds;
      return false;
  }
  if (rlim.rlim_cur < total_fds) {
    rlim.rlim_cur = total_fds;
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1) {
      LOG(ERROR) << "setrlimit error: " << strerror(errno);
      return false;
    }
    LOG(INFO) << "set fd soft limit: " << rlim.rlim_cur;
  }
  return true;
}

int Listen(uint16_t port, int backlog) {
  int lfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
  if (lfd == -1) {
    LOG(ERROR) << "socket error: " << strerror(errno);
    return -1;
  }
  if (!SetReuseAddr(lfd)) {
    Close(lfd);
    return -1;
  }
  sockaddr_in laddr;
  laddr.sin_family = AF_INET;
  laddr.sin_port = htons(port);
  laddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(lfd, reinterpret_cast<sockaddr*>(&laddr), sizeof(laddr)) == -1) {
    LOG(ERROR) << "bind error: " << strerror(errno);
    close(lfd);
    return -1;
  }
  if (listen(lfd, backlog) == -1) {
    LOG(ERROR) << "listen error: " << strerror(errno);
    close(lfd);
    return -1;
  }
  return lfd;
}

int Accept(int sockfd, sockaddr_in& addr) {
  int fd;
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  do {
    fd = accept4(sockfd, reinterpret_cast<sockaddr*>(&addr), &addrlen,
        SOCK_CLOEXEC | SOCK_NONBLOCK);
  } while (fd == -1 && errno == EINTR);
  if (fd == -1) {
    LOG(ERROR) << "accept4 error: " << strerror(errno);
    return -1;
  }
  return fd;
}

int Connect(const char* host, const uint16_t port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    LOG(ERROR) << "socket error: " << strerror(errno);
    return -1;
  }
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); 
  if(inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
    LOG(ERROR) << "invalid host: " << host;
    close(fd);
    return -1;
  }
  if(connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
    LOG(ERROR) << "connect error: " << strerror(errno);
    close(fd);
    return -1;
  }
  return fd;
}

bool SetReuseAddr(int sockfd) {
  int on = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    LOG(ERROR) << "setsockopt error: " << strerror(errno);
    return false;
  }
  return true;
}

bool Close(int fd) {
  if (fd == -1) {
    return true;
  }
  if (close(fd) == -1) {
    LOG(ERROR) << "close error: " << strerror(errno);
    return false;
  }
  return true;
}

int EpollCreate() {
  int efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd == -1) {
    LOG(ERROR) << "epoll_create1 error: " << strerror(errno);
    return -1;
  }
  return efd;
}

bool EpollCtl(int efd, int fd, int op, int events) {
  epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(efd, op, fd, &ev) == -1) {
    LOG(ERROR) << "epoll_ctl error: " << strerror(errno);
    return false;
  }
  return true;
}

int EpollWait(int efd, epoll_event* events, int max_events) {
  int nfds;
  do {
    nfds = epoll_wait(efd, events, max_events, -1);
  } while (nfds == -1 && errno == EINTR);
  if (nfds == -1) {
    LOG(ERROR) << "epoll_wait error: " << strerror(errno);
  }
  return nfds;
}

int EventfdCreate(uint64_t init_val) {
  // TODO(litao.sre): should EFD_NONBLOCK?
  int fd = eventfd(init_val, EFD_CLOEXEC);
  if (fd == -1) {
    LOG(ERROR) << "eventfd error: " << strerror(errno);
    return -1;
  }
  return fd;
}

bool EventfdWrite(int fd, uint64_t val) {
  if (eventfd_write(fd, val) == -1) {
    LOG(ERROR) << "eventfd_write error: " << strerror(errno);
    return false;
  }
  return true;
}

uint64_t EventfdRead(int fd) {
  eventfd_t data;
  if (eventfd_read(fd, &data) == -1) {
    LOG(ERROR) << "eventfd_read error: " << strerror(errno);
    return -1;
  }
  return data;
}

int TimerfdCreate(uint64_t interval_ms) {
  // TODO(litao.sre): should TFD_NONBLOCK?
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
  if (fd == -1) {
    LOG(ERROR) << "timerfd_create error: " << strerror(errno);
    return -1;
  }
  itimerspec its;
  its.it_interval.tv_sec = interval_ms * 1000000L / 1000000000L;
  its.it_interval.tv_nsec = interval_ms * 1000000L % 1000000000L;
  its.it_value = its.it_interval;
  if (timerfd_settime(fd, 0, &its, nullptr) == -1) {
    LOG(ERROR) << "timerfd_settime error: " << strerror(errno);
    close(fd);
    return -1;
  }
  return fd;
}

uint64_t TimerfdRead(int fd) {
  uint64_t expirations;
  // TODO(litao.sre): when error?
  read(fd, &expirations, sizeof(expirations));
  return expirations;
}

}  // namespace hera
