#pragma once

#include <arpa/inet.h>
#include <sys/epoll.h>

namespace hera {

bool SetRlimitNofile(int fds);

int Listen(uint16_t port, int backlog);
int Accept(int sockfd, sockaddr_in& addr);

bool SetReuseAddr(int sockfd);

bool Close(int fd);

int EpollCreate();
bool EpollCtl(int efd, int fd, int op, int events);
int EpollWait(int efd, epoll_event* events, int max_events);

}  // namespace hera
