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
std::map <string, bool> fed_offers_filter_table;

void* PollGossiper(void*);


class FedCommunication : public Anonymous
{
    public:
        FedCommunication();
        virtual ~FedCommunication();
};

#endif // __MESOS_ANONYMOUS_COMMUNICATION_HPP__
