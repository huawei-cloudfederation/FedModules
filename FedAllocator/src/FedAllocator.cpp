#include <unistd.h>
#include "FedAllocator.hpp"
#include <glog/logging.h>


// Forward declararions
void* WaitForFilterUpdate(void* arg);


FederationAllocatorProcess::FederationAllocatorProcess()
{
  threadId = 0;
  InitilizeThread();
}


FederationAllocatorProcess::~FederationAllocatorProcess()
{
  LOG(INFO) << "FEDERATION: Allocator Process Will End Now (Destructor called)";

  pthread_exit(NULL);
  pthread_mutex_destroy(&mutexFedOfferSuppressTable);
  pthread_mutex_destroy(&mutexCondVarForFed);
  pthread_cond_destroy(&condVarForFed);
}


void* WaitForFilterUpdate(void* arg)
{
  FederationAllocatorProcess *obj = (FederationAllocatorProcess*)arg;
  obj->ApplyFilters();
}


void FederationAllocatorProcess::ApplyFilters()
{
  LOG(INFO) << "FEDERATION: ApplyFilters method called";

  // Read the gossiper table and call the suppressOffers OR reviveOffers accordingly.
  while (1)
  {
    // wait for cond var to be signalled
    pthread_mutex_lock(&mutexCondVarForFed);
    pthread_cond_wait(&condVarForFed, &mutexCondVarForFed);

    LOG(INFO) << "FEDERATION: Received update from Fed Communicator";

    if(fedOfferSuppressTable.size() == 0)
	LOG(WARNING) << "FEDERATION: No Framework is registered";
    else
	LOG(INFO) << "FEDERATION: Total Number of FRAMEWORKS registered = " << fedOfferSuppressTable.size();

    //for (map<string, Suppress_T>::iterator it = fedOfferSuppressTable.begin(); it!=fedOfferSuppressTable.end(); ++it)
    for(auto& fedTableData : fedOfferSuppressTable)
    {
      string frmwkId = fedTableData.first;
      bool suppressByFedFlag = fedTableData.second.supByFederationFlag;
      bool suppressByFrmFlag = fedTableData.second.supByFrameworkFlag;
      mesos::FrameworkID fwId = fedTableData.second.frameworkId;
      //string frmwkId = it->first;
      //bool suppressByFedFlag = it->second.supByFederationFlag;
      //bool suppressByFrmFlag = it->second.supByFrameworkFlag;
      //mesos::FrameworkID fwId = it->second.frameworkId;

      LOG(INFO) << "FEDERATION: Apply Filter to Framework ID: " << frmwkId << "\n"
        << "\t\t\tTD: Suppressed by Federation : " << suppressByFedFlag << "\tTD: Suppressed by Framework : " << suppressByFrmFlag << "\n"
        << "\t\t\tSuppress Flag Value in MESOS : " << frameworks[fwId].suppressed;

      bool suppress = (suppressByFedFlag || suppressByFrmFlag);

      // Call Suppress/Revive ONLY IF its NOT already Suppressed/Revived
      if (suppress ^ frameworks[fwId].suppressed)
      {
        if (suppress)
        {
          HierarchicalDRFAllocatorProcess::suppressOffers(fwId);
          LOG(INFO) << "FEDERATION: Suppresed framework Id: " << frmwkId;
        }
        else
        {
          HierarchicalDRFAllocatorProcess::reviveOffers(fwId);
          LOG(INFO) << "FEDERATION: Revived framework Id: " << frmwkId;
        }
      }
    } // for LOOP ends here

    pthread_mutex_unlock(&mutexCondVarForFed);
  } // while LOOP ends here
}


void FederationAllocatorProcess::addFramework(
    const FrameworkID& frameworkId,
    const FrameworkInfo& frameworkInfo,
    const hashmap<SlaveID, Resources>& used)
{
  LOG(INFO) << "FEDERATION: addFramework method is called";

  pthread_mutex_lock(&mutexFedOfferSuppressTable);

  fedOfferSuppressTable[frameworkId.value()].frameworkId = frameworkId;
  fedOfferSuppressTable[frameworkId.value()].supByFrameworkFlag = false;

  pthread_mutex_unlock(&mutexFedOfferSuppressTable);

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::addFramework(frameworkId, frameworkInfo, used);
}


void FederationAllocatorProcess::removeFramework(const FrameworkID& frameworkId)
{
  LOG(INFO) << "FEDERATION: removeFramework method is called";

  pthread_mutex_lock(&mutexFedOfferSuppressTable);

  fedOfferSuppressTable.erase(frameworkId.value());

  pthread_mutex_unlock(&mutexFedOfferSuppressTable);

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::removeFramework(frameworkId);
}


void FederationAllocatorProcess::suppressOffers(const FrameworkID& frameworkId)
{
  CHECK(initialized);
  LOG(INFO) << "FEDERATION: suppressOffers method called " << frameworkId;

  pthread_mutex_lock(&mutexFedOfferSuppressTable);

  fedOfferSuppressTable[frameworkId.value()].supByFrameworkFlag = true;

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::suppressOffers(frameworkId);

  pthread_mutex_unlock(&mutexFedOfferSuppressTable);
}


void FederationAllocatorProcess::reviveOffers(const FrameworkID& frameworkId)
{
  LOG(INFO) << "FEDERATION: reviveOffers method called " << frameworkId;

  pthread_mutex_lock(&mutexFedOfferSuppressTable);

  fedOfferSuppressTable[frameworkId.value()].supByFrameworkFlag = false;

  // Call the parent class method ONLY if federation doesn't say 'suppress'
  if(fedOfferSuppressTable[frameworkId.value()].supByFederationFlag == false)
  {
    HierarchicalDRFAllocatorProcess::reviveOffers(frameworkId);
  }

  pthread_mutex_unlock(&mutexFedOfferSuppressTable);
}


void FederationAllocatorProcess::InitilizeThread()
{
  int tStatus =  pthread_create(&threadId, NULL, WaitForFilterUpdate, this);

  if(tStatus > 0)
  {
    LOG(ERROR) << "Error: Thread creation is unsuccessfull";
  }

}


static Allocator* createFederationAllocator(const Parameters& parameters)
{
  LOG(INFO) << "FEDERATION: createAllocator()";

  Try<Allocator*> allocator = FederationAllocator::create();

  if (allocator.isError())
  {
     return NULL;
  }

  auto alocObj = allocator.get();

  return alocObj;
}


mesos::modules::Module<Allocator> mesos_fed_allocator_module(
  MESOS_MODULE_API_VERSION,
  MESOS_VERSION,
  "Huawei Mesos Federation Project",
  "parushuram.k@huawei.com",
  "Cloud Federation Allocator Module.",
  NULL,
  createFederationAllocator);

