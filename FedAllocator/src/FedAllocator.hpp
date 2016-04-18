#ifndef __MESOS_FEDERATION_ALLOCATOR__
#define __MESOS_FEDERATION_ALLOCATOR__

//#include "../../Fed_Common.hpp"

#include <mesos/module/allocator.hpp>
#include "master/allocator/mesos/hierarchical.hpp"

#include "../../Fed_Common.hpp"

using namespace mesos::internal::master::allocator;
using mesos::master::allocator::Allocator;

// Conditional Variable to get to know the table update
extern pthread_cond_t           condVarForFed; 
// Associated mutex variable for Conditional Variable
extern pthread_mutex_t          mutexCondVarForFed; 

// Table of Suppressed Offers for a Framework by Federation
extern std::map <string, Suppress_T>  fedOfferSuppressTable; 
// Mutex Variable to lock the Suppressed Offer Table
extern pthread_mutex_t          mutexFedOfferSuppressTable; 


// Inherited/Extended Hierarchical DRF Allocator class
class FederationAllocatorProcess : public HierarchicalDRFAllocatorProcess
{
public:
  FederationAllocatorProcess();

  virtual ~FederationAllocatorProcess(); 

  void addFramework(const FrameworkID& frameworkId,
    const FrameworkInfo& frameworkInfo,
    const hashmap<SlaveID, Resources>& used);

  void removeFramework(const FrameworkID& frameworkId);

  void suppressOffers(const FrameworkID& frameworkId);

  void reviveOffers(const FrameworkID& frameworkId);

  void ApplyFilters();

  void InitilizeThread();

private:
  pthread_t threadId;

};


typedef MesosAllocator<FederationAllocatorProcess> FederationAllocator;

#endif // __MESOS_FEDERATION_ALLOCATOR__

