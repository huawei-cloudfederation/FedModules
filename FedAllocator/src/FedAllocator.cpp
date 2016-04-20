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
}


void* WaitForFilterUpdate(void* arg)
{
  FederationAllocatorProcess *obj = (FederationAllocatorProcess*)arg;
  obj->ApplyFilters();
}


void FederationAllocatorProcess::ApplyFilters()
{
  LOG(INFO) << "FEDERATION: Fed Alloc Thread is Created";

  // Read the gossiper table and call the suppressOffers OR reviveOffers accordingly.
  while (1)
  {
    // wait for cond var to be signalled
    mutexCondVarForFed.lock();
    condVarForFed.wait(mutexCondVarForFed);

    LOG(INFO) << "FEDERATION: Received update signal from Fed Communicator";

    if(fedOfferSuppressTable.size() == 0)
	LOG(WARNING) << "FEDERATION: No Framework is registered";
    else
	LOG(INFO) << "FEDERATION: Total Number of FRAMEWORKs registered = " << fedOfferSuppressTable.size();

    mutexFedOfferSuppressTable.lock(); // mutex lock for Table

    for(auto& fedTableData : fedOfferSuppressTable)
    {
      std::string frmwkId = fedTableData.first;
      bool suppressByFedFlag = fedTableData.second.supByFederationFlag;
      bool suppressByFrmFlag = fedTableData.second.supByFrameworkFlag;
      mesos::FrameworkID fwId = fedTableData.second.frameworkId;

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

    mutexFedOfferSuppressTable.unlock();

    mutexCondVarForFed.unlock();
  } // while LOOP ends here
}


// Extended Add Framework Method, which is invoked by Mesos Master
void FederationAllocatorProcess::addFramework(
    const FrameworkID& frameworkId,
    const FrameworkInfo& frameworkInfo,
    const hashmap<SlaveID, Resources>& used)
{
  //mutexFedOfferSuppressTable.lock();

  fedOfferSuppressTable[frameworkId.value()].frameworkId = frameworkId;
  fedOfferSuppressTable[frameworkId.value()].supByFrameworkFlag = false;

  //mutexFedOfferSuppressTable.unlock();

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::addFramework(frameworkId, frameworkInfo, used);

  LOG(INFO) << "FEDERATION: New Framework [ " << frameworkId.value() << " ] Added to MESOS";
}


// Extended Remove Framework Method, which is invoked by Mesos Master
void FederationAllocatorProcess::removeFramework(const FrameworkID& frameworkId)
{
  mutexFedOfferSuppressTable.lock();

  fedOfferSuppressTable.erase(frameworkId.value());

  mutexFedOfferSuppressTable.unlock();

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::removeFramework(frameworkId);

  LOG(INFO) << "FEDERATION: Framework [ " << frameworkId.value() << " ] Removed From MESOS";
}


// Extended Suppress Offer Method, which is invoked by Framework
void FederationAllocatorProcess::suppressOffers(const FrameworkID& frameworkId)
{
  CHECK(initialized);

  mutexFedOfferSuppressTable.lock();

  fedOfferSuppressTable[frameworkId.value()].supByFrameworkFlag = true;

  // Call the parent class method
  HierarchicalDRFAllocatorProcess::suppressOffers(frameworkId);

  mutexFedOfferSuppressTable.unlock();

  LOG(INFO) << "FEDERATION: Offer Suppressed by Framework :  " << frameworkId;
}


// Extended Revive Offer Method, which is invoked by Framework
void FederationAllocatorProcess::reviveOffers(const FrameworkID& frameworkId)
{
  mutexFedOfferSuppressTable.lock();

  fedOfferSuppressTable[frameworkId.value()].supByFrameworkFlag = false;

  // Call the parent class method ONLY if federation doesn't say 'suppress'
  if(fedOfferSuppressTable[frameworkId.value()].supByFederationFlag == false)
  {
    HierarchicalDRFAllocatorProcess::reviveOffers(frameworkId);
  }

  mutexFedOfferSuppressTable.unlock();

  LOG(INFO) << "FEDERATION: Offer Revived by Framework :  " << frameworkId;
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
  Try<Allocator*> allocator = FederationAllocator::create();

  if (allocator.isError())
  {
     return NULL;
  }

  auto alocObj = allocator.get();

  LOG(INFO) << "FEDERATION: Fed Allocator Module created, and will be loaded to mesos";
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

