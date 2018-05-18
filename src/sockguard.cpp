#include "sockguard.h"

#include <sys/unistd.h>

SockGuard::SockGuard(int sock):
  sock_(sock)
{}

SockGuard::~SockGuard()
{
  close(sock_);
}
