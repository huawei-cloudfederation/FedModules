#include <unistd.h>

#include "FedAllocator.hpp"


// Forward declararions
void* WaitForFilterUpdate(void* arg);


FederationAllocatorProcess::FederationAllocatorProcess()
{
	cout << "========== HUAWEI - FederationAllocatorProcess Constructor" << endl;

	pthread_t threadId_WaitForFilterUpdate;
	pthread_create(&threadId_WaitForFilterUpdate, NULL, WaitForFilterUpdate, this);
}

void* WaitForFilterUpdate(void* arg)
{
	cout << "========== HUAWEI - WaitForFilterUpdate Thread " <<endl;

	FederationAllocatorProcess *obj = (FederationAllocatorProcess*)arg;
	obj->ApplyFilters();
}

void FederationAllocatorProcess::
ApplyFilters()
{
	cout << "========== HUAWEI - ApplyFilters method called" << endl;

	// Read the gossiper table and call the suppressOffers OR reviveOffers accordingly.
	while (1)
	{
		// wait for cond var to be signalled


		pthread_mutex_lock(&mutex_fed_offer_suppress_table);

		for (map<string, Suppress_T>::iterator it = fed_offer_suppress_table.begin(); it!=fed_offer_suppress_table.end(); ++it)
		{
			bool suppress_fed = it->second.federation;
			bool suppress_frm = it->second.framework;
			mesos::FrameworkID framework_id = it->second.framework_id;

			bool suppress = (suppress_fed || suppress_frm);

			// Call Suppress/Revive ONLY IF its NOT already Suppressed/Revived
			if (suppress ^ frameworks[framework_id].suppressed)
			{
				if (suppress)
				{
					HierarchicalDRFAllocatorProcess::suppressOffers(framework_id);
				}
				else
				{
					HierarchicalDRFAllocatorProcess::reviveOffers(framework_id);
				}
			}
		}
		
		pthread_mutex_unlock(&mutex_fed_offer_suppress_table);
	}
}

void FederationAllocatorProcess::
addFramework(const FrameworkID& frameworkId,
            const FrameworkInfo& frameworkInfo, 
            const hashmap<SlaveID, Resources>& used)
{
    cout << "========== HUAWEI - addFramework method is called" << endl;

    pthread_mutex_lock(&mutex_fed_offer_suppress_table);

    fed_offer_suppress_table[frameworkId.value()].framework_id = frameworkId;
    fed_offer_suppress_table[frameworkId.value()].framework = false;

    pthread_mutex_unlock(&mutex_fed_offer_suppress_table);

    // Call the parent class method
    HierarchicalDRFAllocatorProcess::addFramework(frameworkId, frameworkInfo, used);
}

void FederationAllocatorProcess::
removeFramework(const FrameworkID& frameworkId)
{
    cout << "========== HUAWEI - removeFramework method is called" << endl;

    pthread_mutex_lock(&mutex_fed_offer_suppress_table);

    fed_offer_suppress_table.erase(frameworkId.value());

    pthread_mutex_unlock(&mutex_fed_offer_suppress_table);

    // Call the parent class method
    HierarchicalDRFAllocatorProcess::removeFramework(frameworkId);
}

void FederationAllocatorProcess::
suppressOffers(const FrameworkID& frameworkId)
{
    CHECK(initialized);
    cout << "========== HUAWEI - " << "suppressOffers method called " << frameworkId << endl;
    
    pthread_mutex_lock(&mutex_fed_offer_suppress_table);

    fed_offer_suppress_table[frameworkId.value()].framework = true;

    // Call the parent class method
    HierarchicalDRFAllocatorProcess::suppressOffers(frameworkId);

    pthread_mutex_unlock(&mutex_fed_offer_suppress_table);
}

void FederationAllocatorProcess::
reviveOffers(const FrameworkID& frameworkId)
{
    cout << "========== HUAWEI - " << "reviveOffers method called " << frameworkId << endl;

    pthread_mutex_lock(&mutex_fed_offer_suppress_table);

    fed_offer_suppress_table[frameworkId.value()].framework = false;

    // Call the parent class method ONLY if federation doesn't say 'suppress'
    if(fed_offer_suppress_table[frameworkId.value()].federation == false)
    {
        HierarchicalDRFAllocatorProcess::reviveOffers(frameworkId);
    }

    pthread_mutex_unlock(&mutex_fed_offer_suppress_table);
}

static Allocator* createFederationAllocator(const Parameters& parameters)
{
    cout << "========== HUAWEI - createAllocator()" << endl;

    Try<Allocator*> allocator = FederationAllocator::create();

    if (allocator.isError()) 
    {
       return NULL;
    }

    return allocator.get();
}

mesos::modules::Module<Allocator> mesos_fed_allocator_module(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Huawei Mesos Federation Project",
    "parushuram.k@huawei.com",
    "Cloud Federation Allocator Module.",
    NULL,
    createFederationAllocator);

