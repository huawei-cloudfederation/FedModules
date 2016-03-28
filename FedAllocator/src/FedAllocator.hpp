#ifndef __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_FEDALLOC_HPP__
#define __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_FEDALLOC_HPP__

#include <mesos/module/allocator.hpp>
#include "master/allocator/mesos/hierarchical.hpp"
#include <stout/try.hpp>

#include <pthread.h>

using namespace std;
using namespace mesos;
using namespace mesos::internal::master::allocator;

using mesos::master::allocator::Allocator;


extern int fed_shared_var;
extern pthread_mutex_t mutex_fed_offers_filter_table;
//extern "C" __attribute__ ((visibility ("default"))) int fed_get(void);


class FederationAllocatorProcess : public HierarchicalDRFAllocatorProcess 
{
    public:
        FederationAllocatorProcess();

        virtual ~FederationAllocatorProcess() {}

        virtual void addFramework(const FrameworkID& frameworkId,
                                    const FrameworkInfo& frameworkInfo,
                                    const hashmap<SlaveID, Resources>& used);
};


typedef MesosAllocator<FederationAllocatorProcess> FederationAllocator;

#endif // __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_FEDALLOC_HPP__
