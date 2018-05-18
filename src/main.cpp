#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <netinet/in.h>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>

#include "db_manager.h"
#include "sockguard.h"
#include "processed.h"

int main()
{
  try
  {
    DataBaseManager* db = &DataBaseManager::instance();
    int listener;
    sockaddr_in addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
      exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8003);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
      exit(2);
    }

    listen(listener, 1);

    while(1)
    {
      SockGuard sock(accept(listener, nullptr, nullptr));
      if(sock.sock_ < 0)
      {
        exit(3);
      }

      handle(sock.sock_);
    }
  }
  catch(int i)
  {
    std::cerr << i;
  }

  return 0;
}
