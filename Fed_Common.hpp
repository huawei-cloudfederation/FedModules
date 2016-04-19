#ifndef __MESOS_FEDERATION_COMMON__
#define __MESOS_FEDERATION_COMMON__

#include <pthread.h>

#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <map>
#include <stdlib.h>
#include <mutex>
#include <condition_variable>

//using namespace std;
using namespace mesos;

typedef struct
{
  mesos::FrameworkID frameworkId; // To store the framework id of registered framework (to mesos)
  bool supByFrameworkFlag;                 // Flag: Suppressed by Framwork
  bool supByFederationFlag;                // Flag: Suppressed by Federation
} Suppress_T;


#endif //__MESOS_FEDERATION_COMMON__
