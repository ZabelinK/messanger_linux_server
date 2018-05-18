#include "processed.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <netinet/in.h>

#include <cstdlib>
#include <vector>
//Убрать нижние
#include <iterator>
#include <iostream>

#include "answer.h"

void handle(int sock)
{
  std::string request;
  char buf[1024];
  size_t size_read = 1024;
  while(size_read == 1024)
  {
    size_read = recv(sock, buf, 1024, 0);
    request += std::string(buf, buf + size_read);
    if(size_read < 1024)
    {
      break;
    }
  }
  std::cout << "2" << std::endl;

  std::cout << request << '\n' << "!" << std::endl;
  std::string answ(answer(request));
  std::cout << answ << '\n';
  send(sock, answ.c_str(), answ.size(), 0);
}
