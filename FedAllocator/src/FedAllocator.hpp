#ifndef __MESOS_FEDERATION_ALLOCATOR__
#define __MESOS_FEDERATION_ALLOCATOR__

#include <mesos/module/allocator.hpp>
#include "master/allocator/mesos/hierarchical.hpp"

#include "../../Fed_Common.hpp"


using namespace mesos::internal::master::allocator;
using mesos::master::allocator::Allocator;


/*
Extern declaraions
*/
// Condition-Variable used for signaling the update in fedOfferSuppressTable
// and the Mutex associated with Condition-Variable
extern std::condition_variable  condVarForFed;
extern std::mutex               CondVarForFed;

// Table of Suppress-offers info for the registered Frameworks
extern std::map <std::string, Suppress_T>   fedOfferSuppressTable; 
// Mutex to lock the fedOfferSuppressTable 
extern std::mutex                           FedOfferSuppressTable; 


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

