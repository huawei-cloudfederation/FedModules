#ifndef __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_FEDALLOC_HPP__
#define __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_FEDALLOC_HPP__

#include <mesos/mesos.hpp>
#include <mesos/module.hpp>

#include <mesos/master/allocator.hpp>
#include <mesos/module/allocator.hpp>

#include "master/allocator/mesos/hierarchical.hpp"
#include "master/allocator/sorter/drf/sorter.hpp"

#include "master/constants.hpp"
#include <stout/try.hpp>


using namespace std;
using namespace mesos;
using mesos::master::allocator::Allocator;
using mesos::internal::master::allocator::HierarchicalDRFAllocator;
using mesos::internal::master::allocator::DRFSorter;
using mesos::internal::master::allocator::MesosAllocator;
using mesos::internal::master::allocator::HierarchicalAllocatorProcess;


class FederationAllocator;
typedef HierarchicalAllocatorProcess<DRFSorter, DRFSorter> HierarchicalDRFAllocatorProcess;
typedef MesosAllocator<FederationAllocator> FilterOffersFedAlloc;

extern int fed_shared_var;
//extern "C" __attribute__ ((visibility ("default"))) int fed_get(void);

class FederationAllocator : public HierarchicalDRFAllocatorProcess 
{
    public:
        FederationAllocator();

        virtual ~FederationAllocator() {}

        virtual void addFramework(const FrameworkID& frameworkId,
                                    const FrameworkInfo& frameworkInfo,
                                    const hashmap<SlaveID, Resources>& used);
};

#endif // __MASTER_ALLOCATOR_MESOS_HIERARCHICAL_FEDALLOC_HPP__
