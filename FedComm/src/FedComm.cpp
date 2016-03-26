#include <pthread.h>
#include <unistd.h>

#include "FedComm.hpp"


FedCommunication :: FedCommunication()
{
    cout << "========== HUAWEI - FedCommunication Constructor" << endl;
    cout << "========== HUAWEI - " << "fed_shared_var: " << fed_shared_var <<"   "  << &fed_shared_var  << endl;
    fed_shared_var = 13;
    //cout << "========== HUAWEI - " << "fed_get() : " << fed_get() << endl;
    
    pthread_t threadId_poll_gosipper;
    pthread_create(&threadId_poll_gosipper, NULL, PollGossiper, NULL); 
}

FedCommunication :: ~FedCommunication()
{
    cout << "========== HUAWEI - FedCommunication Destructor" << endl;
}

void* PollGossiper(void* arg)
{
    pthread_mutex_t mutex_fed_offers_filter_table = PTHREAD_MUTEX_INITIALIZER;
    while (1)
    {
        cout << "test Poll Gossiper" <<endl;
        ++fed_shared_var;
        sleep (1);

        // Poll the gossiper

        // Updatet the table with values read from gossiper
        fed_offers_filter_table;
    }
}

static Anonymous* createFedCommunicator(const Parameters& parameters)
{
    auto obj = new FedCommunication();
    return obj;
}

// Declares an Anonymous module named 'mesos_fed_comm_module'.
mesos::modules::Module<Anonymous> mesos_fed_comm_module(
    MESOS_MODULE_API_VERSION,
    MESOS_VERSION,
    "Huawei Mesos Federation",
    "parushuram.k@huawei.com",
    "Mesos Federation Communication Module.",
    NULL,
    createFedCommunicator);

