#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "FedUtil.hpp"

#include <glog/logging.h>


extern int ConnectToGossiper();

int ReadConfig(Config &cfg)
{
  string port;
  ifstream file("fedconf.cfg", ifstream::binary);
  if (!file)
  {
    LOG(INFO) << "ERROR";
    return 0;
  }
  if(!file.eof())
  {
    file >> cfg.gossiper_ip;
    file >> port;
    cfg.gossiper_port = atoi(port.c_str());
  }

  file.close();
  return 1;
}

int Fed_Read(int& fd, char* buf, int cnt)
{
  int n;
  n = read(fd, buf, cnt);

  if(n < 0)
  {
    LOG(ERROR) << "ERROR reading from socket";
    fd = ConnectToGossiper();
    if (!fd)
      return 0;
  }
  else
  {
    return 1;
  }
}

