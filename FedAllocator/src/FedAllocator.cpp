#include <unistd.h>

#include "FedAllocator.hpp"

#include <glog/logging.h>


// Forward declararions
void* WaitForFilterUpdate(void* arg);


FederationAllocatorProcess::FederationAllocatorProcess()
{
  LOG(INFO) << "FederationAllocatorProcess Constructor";

  pthread_t threadId_WaitForFilterUpdate;
  pthread_create(&threadId_WaitForFilterUpdate, NULL, WaitForFilterUpdate, this);
}

void* WaitForFilterUpdate(void* arg)
{
  LOG(INFO) << "WaitForFilterUpdate Thread ";

  FederationAllocatorProcess *obj = (FederationAllocatorProcess*)arg;
  obj->ApplyFilters();
}

void FederationAllocatorProcess::
ApplyFilters()
{
  LOG(INFO) << "ApplyFilters method called";

  // Read the gossiper table and call the suppressOffers OR reviveOffers accordingly.
  while (1)
  {
    // wait for cond var to be signalled
    pthread_mutex_lock(&mutex_fed_offer_suppress_table);
    pthread_cond_wait(&cond_var_filter, &mutex_fed_offer_suppress_table);

    LOG(INFO) << "Received update from Communicator";
    for (map<string, Suppress_T>::iterator it = fed_offer_suppress_table.begin(); it!=fed_offer_suppress_table.end(); ++it)
    {
      string f_id = it->first;
      bool suppress_fed = it->second.federation;
      bool suppress_frm = it->second.framework;
      mesos::FrameworkID framework_id = it->second.framework_id;

      LOG(INFO) << "processing for Framework ID: " << f_id;
      LOG(INFO) <<"\tsuppress_fed : " << suppress_fed <<"\tsuppress_frm : " << suppress_frm;
      LOG(INFO) <<"\tframeworks[framework_id].suppressed : " << frameworks[framework_id].suppressed;

      bool suppress = (suppress_fed || suppress_frm);

      //LOG(INFO) << f_id <<" : " << suppress_fed;
      // Call Suppress/Revive ONLY IF its NOT already Suppressed/Revived
      if (suppress ^ frameworks[framework_id].suppressed)
      {
        if (suppress)
        {
          LOG(INFO) << "Suppresed framework Id: " << f_id;
          HierarchicalDRFAllocatorProcess::suppressOffers(framework_id);
        }
        else
        {
          LOG(INFO) << "Revived framework Id: " << f_id;
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
  LOG(INFO) << "addFramework method is called";

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
  LOG(INFO) << "removeFramework method is called";

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
  LOG(INFO) << "" << "suppressOffers method called " << frameworkId;

  pthread_mutex_lock(&mutex_fed_offer_suppress_table);

  fed_offer_suppress_table[frameworkId.value()].framework = true;

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::suppressOffers(frameworkId);

  pthread_mutex_unlock(&mutex_fed_offer_suppress_table);
}

void FederationAllocatorProcess::
reviveOffers(const FrameworkID& frameworkId)
{
  LOG(INFO) << "" << "reviveOffers method called " << frameworkId;

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
  LOG(INFO) << "createAllocator()";

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

