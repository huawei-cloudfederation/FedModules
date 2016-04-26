#include "FedComm.hpp"
#include <glog/logging.h>

/*
Constructor
*/
FedCommunication :: FedCommunication()
{
  fedCommSockfd = -1; 
  portno = 5555; // default port num from Gossiper to Fed Comm
  threadId = 0;
}


/*
Destructor
*/
FedCommunication :: ~FedCommunication()
{
  LOG(INFO) << "FEDERATION: FedCommunication Closing Now (Destructor called)";

  // cleanup activity
  pthread_exit(NULL);
  close(fedCommSockfd);
}


/*
*/
void FedCommunication::OpenServerSocket()
{
  fedCommSockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (fedCommSockfd < 0)
  {
    LOG(ERROR) << "FEDERATION-Error: Opening the fedComm machine socket";
  }

  // Initialize socket structure
  bzero((char *) &fedComm_addr, sizeof(fedComm_addr));

  fedComm_addr.sin_family = AF_INET;
  fedComm_addr.sin_addr.s_addr = INADDR_ANY;
  fedComm_addr.sin_port = htons(portno);

  // Now bind the host address
  if (bind(fedCommSockfd, (struct sockaddr *) &fedComm_addr, sizeof(fedComm_addr)) < 0)
  {
    LOG(ERROR) << "FEDERATION-Error: on Binding";
    return;
  }

  LOG(INFO) << "FEDERATION: Server Socket(" << fedCommSockfd << ") Opened and Binded Successfully";

  // Create a thread to accept the client - Gossiper connection
  int tStatus = pthread_create(&threadId, NULL, PollFedGossiper, (void*) &fedCommSockfd);

  if(tStatus > 0)
  {
    LOG(ERROR) << "FEDERATION-Error: Thread creation unsuccessfull";
    return;
  }

  LOG(INFO) << "FEDERATION: Thread created and running successfully";
}


// Parser for gossiper message
// If there is any change in the framework (Flag value in table), 
// then only it make sense to send signal to Fed Alloc Module
bool ParseGossiperMessage(char* gossiper_info)
{
  std::stringstream str(gossiper_info);
  std::stringstream json;
  std::string token;
  bool updated = false;

  json << "[";

  std::unique_lock <std::mutex> mutexFedOfferSuppressTable(FedOfferSuppressTable);

  while (str >> token)
  {
    int sep = token.find(':');
    std::string fId(token, 0, sep);

    bool fedFlag;
    if (fedOfferSuppressTable.find(fId) != fedOfferSuppressTable.end())
    {
      fedFlag = (token[sep+1] == '1') ? true : false;
      // Only if federated framework is modified, will set updated flag to true
      if(fedOfferSuppressTable[fId].supByFederationFlag != fedFlag) updated = true;

      fedOfferSuppressTable[fId].supByFederationFlag = fedFlag;
    }

    json << " { " << fId << " : " << fedFlag << " } ";
  }

  mutexFedOfferSuppressTable.unlock(); // mutex unlock for Table

  json << "]";
  if(updated) LOG(INFO) << "FEDERATION: Parsed Gossiper Info ==> " << json.str();

  return updated;
}


/*
*/
void ParseGossiperMsgSendSignal(int gossiperSockfd, unsigned int MsgLen, bool &notFirstTime)
{
  char* gossiper_info = new char[MsgLen];
  int numChar = read(gossiperSockfd, gossiper_info, MsgLen);

  std::unique_lock <std::mutex> mutexCondVarForFed(CondVarForFed);

  bool isUpdated = ParseGossiperMessage(gossiper_info);

  delete [] gossiper_info;

  if(fedOfferSuppressTable.size() > 0)
  {
     // Check if the table is modified or not
     // depends on this send the signal to Fed Alloc module
     if(isUpdated)
     {
        LOG(INFO) << "FEDERATION: Now table update signal will be sent to Fed Alloc";
        mutexCondVarForFed.unlock(); // mutex unlock for Table
        condVarForFed.notify_one();
     }
     notFirstTime = true;
  }

  // This needs to be tested properly, to activated below line, I am not sure now
  //if(!isUpdated) mutexCondVarForFed.unlock(); // mutex unlock for Table
}


/*
*/
void GetFrameworkInfoFromGossiper(int gossiperSockfd)
{
  unsigned int MsgLen, MsgCnt, MsgType, buf;
  unsigned char ack = 2;
  bool notFirstTime = false;

  // Until gossiper is connected to the Fed Comm (mesos)
  while(gossiperSockfd > 0)
  {
    MsgLen = MsgCnt = 0;
    buf = MsgType = 0;

    // Read message type
    int numChar = read(gossiperSockfd, &MsgType, 1);

    if (MsgType == MSG_TYPE_ACK && numChar == 0)
    {
      break; // When there is no response from Gossiper to Fed Comm
    }
    else if (MsgType == MSG_TYPE_HEARTBEAT)
    {
      numChar = write(gossiperSockfd,(void *) &ack,1);
      LOG(INFO) << "FEDERATION: Sent Heart Beat to Gossiper";
    }
    else if (MsgType == MSG_TYPE_DATA) // If data is sent by the Gossiper
    {
       // Read the length of payload
      numChar = read(gossiperSockfd, &buf, 4);
      MsgLen = ntohl(buf);

      // when framework is not running Gossiper is sending zero info
      if(MsgLen <= 0 || numChar <= 0)
      {
        LOG(WARNING) << "FEDERATION: Looks like Framework is NOT running, so Gossiper sending BLANK info";
        continue;
      }

      // For the first time table does not have anything
      if(fedOfferSuppressTable.size() <= 0 && notFirstTime == true)
      {
        LOG(WARNING) << "FEDERATION: Looks like Framework is NOT running, so Gossiper sending OLD info";
        continue;
      }

      // Read the count of frameworks
      numChar = read(gossiperSockfd, &buf, 4);
      MsgCnt = ntohl(buf);

      ParseGossiperMsgSendSignal(gossiperSockfd, MsgLen, notFirstTime);
    }
  } // while LOP ends here
}


/*
Thread function
*/
void* PollFedGossiper(void* servSockfd)
{
  int gossiperSockfd;
  struct sockaddr_in gossiper_addr;
  socklen_t cliLen = sizeof(gossiper_addr);

  // Accept the gossiper connection here
  int fedCommSockfd =  *((int*)servSockfd);

  while(fedCommSockfd > 0)
  {
    int res = listen(fedCommSockfd, 0);

    if(res < 0)
    {
      LOG(ERROR) << "FEDERATION: unable to accept connection";
    }

    gossiperSockfd = accept(fedCommSockfd, (struct sockaddr *) &gossiper_addr, &cliLen);

    char clntName[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &gossiper_addr.sin_addr.s_addr, clntName, sizeof(clntName));
    unsigned int port = ntohs(gossiper_addr.sin_port);

    if(gossiperSockfd > 0)
    {
       LOG(INFO) << "FEDERATION: Gossiper (Machine Name : " << clntName << " and port # " << port << ") established connection with Fed Comm Module" ;
       GetFrameworkInfoFromGossiper(gossiperSockfd);
       LOG(INFO) << "FEDERATION: Connection with Gossiper is LOST, will wait for new connection";
       close(gossiperSockfd);
    }
    else
    {
      LOG(ERROR) << "FEDERATION-Error: Connecting to the gossiper machine " << clntName << " and port # " << port;
    }
  }
}


/*
Anonymous module creation
*/
static Anonymous* createFedCommunicator(const Parameters& parameters)
{
  auto obj = new FedCommunication();
  if( obj != NULL)
  LOG(INFO) << "FEDERATION: Created the module mesos_fed_comm_module";
  obj->OpenServerSocket();
  return obj;
}


// Declares an Anonymous module named 'mesos_fed_comm_module'.
mesos::modules::Module<Anonymous> mesos_fed_comm_module(
  MESOS_MODULE_API_VERSION,
  MESOS_VERSION,
  "Huawei Mesos Federation Project",
  "parushuram.k@huawei.com",
  "Mesos Federation Communication Module.",
  NULL,
  createFedCommunicator);

