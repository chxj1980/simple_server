#pragma once

#include <arpa/inet.h>
#include <sys/epoll.h>

namespace hera {

bool SetRlimitNofile(int fds);

int Listen(uint16_t port, int backlog);
int Accept(int sockfd, sockaddr_in& addr);
int Connect(const char* host, const uint16_t port);

bool SetReuseAddr(int sockfd);

bool Close(int fd);

int EpollCreate();
bool EpollCtl(int efd, int fd, int op, int events);
int EpollWait(int efd, epoll_event* events, int max_events);

int EventfdCreate(uint64_t init_val);
bool EventfdWrite(int fd, uint64_t val);
uint64_t EventfdRead(int fd);

}  // namespace hera
