#ifndef SOCKGUARD_H
#define SOCKGUARD_H


class SockGuard
{
public:
  SockGuard(int);
  ~SockGuard();
  int sock_;
};

#endif // SOCKGUARD_H
