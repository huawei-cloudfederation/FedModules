#include <iostream>
#include <unistd.h>

#include "FedAllocator.hpp"


/*
int fed_get(void)
{
   return fed_shared_var;
}
*/

void testit()
{
    while (1)
    {
        sleep (1);
        
        pthread_mutex_lock(&mutex_fed_offers_filter_table);
        
        cout<<"Test allocation module. fed_shared_var: "<<fed_shared_var <<endl;

        pthread_mutex_lock(&mutex_fed_offers_filter_table);
    }   
}

FederationAllocatorProcess::FederationAllocatorProcess()
{
    ;
}

void FederationAllocatorProcess::addFramework(const FrameworkID& frameworkId, 
                                        const FrameworkInfo& frameworkInfo, 
                                        const hashmap<SlaveID, Resources>& used)
{
    cout << "========== HUAWEI - addFramework method is called" << endl;
    HierarchicalDRFAllocatorProcess::addFramework(frameworkId, frameworkInfo, used);
    cout << "========== HUAWEI - " << "fed_shared_var: " << fed_shared_var <<"   "  << &fed_shared_var  << endl;
    testit();
    //cout << "========== HUAWEI - " << "fed_get() : " << fed_get() << endl;
}

static Allocator* createFederationAllocator(const Parameters& parameters)
{
    cout << "========== HUAWEI - createAllocator()" << endl;
    cout << "========== HUAWEI - " << "fed_shared_var: " << fed_shared_var <<"   "  << &fed_shared_var  << endl;
    //cout << "========== HUAWEI - " << "fed_get() : " << fed_get() << endl;
    
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

