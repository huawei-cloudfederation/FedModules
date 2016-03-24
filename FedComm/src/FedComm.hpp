#ifndef __MESOS_ANONYMOUS_COMMUNICATION_HPP__
#define __MESOS_ANONYMOUS_COMMUNICATION_HPP__ 

#include <iostream>
#include <mesos/mesos.hpp>
#include <mesos/module.hpp>

#include <mesos/module/anonymous.hpp>

#include <stout/foreach.hpp>
#include <stout/os.hpp>
#include <stout/try.hpp>
#include <unistd.h>

using namespace std;
using namespace mesos;
using mesos::modules::Anonymous;

int fed_shared_var;
//extern "C" int fed_get(void);


class FedCommunication : public Anonymous
{
   public:
   FedCommunication()
   {
      cout << "========== HUAWEI - FedCommunication Constructor" << endl;
      cout << "========== HUAWEI - " << "fed_shared_var: " << fed_shared_var <<"   "  << &fed_shared_var  << endl;
      fed_shared_var = 13;
      //cout << "========== HUAWEI - " << "fed_get() : " << fed_get() << endl;
   }

   virtual ~FedCommunication()
   {
      cout << "========== HUAWEI - FedCommunication Destructor" << endl;
   }
};

#endif // __MESOS_ANONYMOUS_COMMUNICATION_HPP__
