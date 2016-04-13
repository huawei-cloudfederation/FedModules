#ifndef __MESOS_FEDERATION_ALLOCATOR__
#define __MESOS_FEDERATION_ALLOCATOR__

#include <mesos/module/allocator.hpp>
#include "master/allocator/mesos/hierarchical.hpp"

#include "../../Fed_Common.hpp"


using namespace mesos::internal::master::allocator;
using mesos::master::allocator::Allocator;


extern pthread_mutex_t          mutexFedOfferSuppressTable;

extern pthread_cond_t           condVarForFed;
extern pthread_mutex_t          mutexCondVarForFed;

extern std::map <string, Suppress_T>  fedOfferSuppressTable;


class FederationAllocatorProcess : public HierarchicalDRFAllocatorProcess
{
public:
  FederationAllocatorProcess();
  virtual ~FederationAllocatorProcess() {}

  void addFramework(const FrameworkID& frameworkId,
    const FrameworkInfo& frameworkInfo,
    const hashmap<SlaveID, Resources>& used);

  void removeFramework(const FrameworkID& frameworkId);

  void suppressOffers(const FrameworkID& frameworkId);
  void reviveOffers(const FrameworkID& frameworkId);

  void ApplyFilters();
};


typedef MesosAllocator<FederationAllocatorProcess> FederationAllocator;

#endif // __MESOS_FEDERATION_ALLOCATOR__

