#ifndef __MESOS_FEDERATION_UTIL__
#define __MESOS_FEDERATION_UTIL__

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>


using namespace std;

typedef struct Config
{
  string gossiper_ip;
  unsigned short gossiper_port;
}Config;


int ReadConfig(Config &cfg);
int Fed_Read(int& fd, char* buf, int cnt);

#endif // __MESOS_FEDERATION_UTIL__

