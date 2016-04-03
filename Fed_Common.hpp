#ifndef __MESOS_FEDERATION_COMMON__
#define __MESOS_FEDERATION_COMMON__

#include <pthread.h>

#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <map>
#include <stdlib.h>


using namespace std;
using namespace mesos;


typedef struct
{
    mesos::FrameworkID framework_id;
    bool framework;
    bool federation;
} Suppress_T;

#endif //__MESOS_FEDERATION_COMMON__
