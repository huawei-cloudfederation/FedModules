#include "FedUtil.hpp"


int ReadConfig(Config &cfg)
{
    string port;
    ifstream file("conf.cfg", ifstream::binary);
    if (!file)
    {
        cout << "ERROR";
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


