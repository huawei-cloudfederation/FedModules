#include "FedCom.hpp"

static Anonymous* createFedCommunicator(const Parameters& parameters)
{
   auto obj = new FedCommunication();
   return obj;
}

// Declares an Anonymous module named 'mesos_fedCom'.
mesos::modules::Module<Anonymous> mesos_fed_comm_module(
   MESOS_MODULE_API_VERSION,
   MESOS_VERSION,
   "Huawei Mesos Federation",
   "parushuram.k@huawei.com",
   "Mesos Federation Communication Module.",
   NULL,
   createFedCommunicator);

