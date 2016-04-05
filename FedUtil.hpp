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

