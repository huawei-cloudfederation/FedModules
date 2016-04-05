#ifndef __MESOS_FEDERATION_ALLOCATOR__
#define __MESOS_FEDERATION_ALLOCATOR__

#include <mesos/module/allocator.hpp>
#include "master/allocator/mesos/hierarchical.hpp"

#include "../../Fed_Common.hpp"


using namespace mesos::internal::master::allocator;
using mesos::master::allocator::Allocator;


extern std::map <string, Suppress_T>    fed_offer_suppress_table;
extern pthread_cond_t                   cond_var_filter;
extern pthread_mutex_t                  mutex_fed_offer_suppress_table;


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
