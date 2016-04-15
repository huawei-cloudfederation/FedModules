#include "FedComm.hpp"
#include <glog/logging.h>


// Constructor
FedCommunication :: FedCommunication()
{
  fedCommSockfd = portno = 0;
}


FedCommunication :: ~FedCommunication()
{
  LOG(INFO) << "FEDERATION: FedCommunication Closing Now (Destructor called)";

  // cleanup activity
  pthread_exit(NULL);
  pthread_mutex_destroy(&mutexFedOfferSuppressTable);
  pthread_mutex_destroy(&mutexCondVarForFed);
  pthread_cond_destroy(&condVarForFed);
}


void FedCommunication::OpenServerSocket()
{
  fedCommSockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (fedCommSockfd < 0)
  {
    LOG(ERROR) << "Error: Opening the fedComm machine socket";
  }

  // Initialize socket structure
  bzero((char *) &fedComm_addr, sizeof(fedComm_addr));

  portno = 5555;

  fedComm_addr.sin_family = AF_INET;
  fedComm_addr.sin_addr.s_addr = INADDR_ANY;
  fedComm_addr.sin_port = htons(portno);

  // Now bind the host address
  if (bind(fedCommSockfd, (struct sockaddr *) &fedComm_addr, sizeof(fedComm_addr)) < 0)
  {
    LOG(ERROR) << "FEDERATION: Error on Binding";
    return;
  }

  LOG(INFO) << "FEDERATION: Server Socket(" << fedCommSockfd << ") Opened iand Binded Successfully";

  // Create a thread to accept the client - Gossiper connection
  int tStatus = pthread_create(&threadId, NULL, PollFedGossiper, (void*) &fedCommSockfd);

  if(tStatus > 0)
  {
    LOG(ERROR) << "Error: Thread creation unsuccessfull";
    return;
  }

  LOG(INFO) << "FEDERATION: Thread ceated and running successfully";
}


// Parser for gossiper messag
bool ParseGossiperMessage(char* gossiper_info)
{
  stringstream str(gossiper_info);
  string token;
  stringstream json;
  bool updated = false;

  json << "[";

  pthread_mutex_lock(&mutexFedOfferSuppressTable);

  while (str >> token)
  {
    int sep = token.find(':');
    string fId(token, 0, sep);

    bool fedFlag;
    if (fedOfferSuppressTable.find(fId) != fedOfferSuppressTable.end())
    {
      fedFlag = (token[sep+1] == '1') ? true : false;
      // Only if federated framework is modified, will set updated flag to true
      if(fedOfferSuppressTable[fId].supByFederationFlag != fedFlag) updated = true;

      fedOfferSuppressTable[fId].supByFederationFlag = fedFlag;
    }

    if(updated) json << " { " << fId << " : " << fedFlag << " } ";
  }

  pthread_mutex_unlock(&mutexFedOfferSuppressTable);

  json << "]";
  LOG(INFO) << "FEDERATION: Parsed Gossiper Info ==> " << json.str();

  return updated;
}


void GetFrameworkInfoFromGossiper(int gossiperSockfd)
{
  unsigned int MsgLen, MsgCnt, MsgType, buf;
  unsigned char ack = 2;
  bool notFirstTime = false;

  LOG(INFO) << "FEDERATION: Client Gossiper sock fd is " << gossiperSockfd << " ";

  // Until gossiper is connected to the Fed Comm (mesos)
  while(gossiperSockfd > 0)
  {
    MsgLen = MsgCnt = 0;
    buf = MsgType = 0;

    // Read message type
    int numChar = read(gossiperSockfd, &MsgType, 1);

    // It's an ACK
    if (MsgType == MSG_TYPE_ACK && numChar == 0)
    {
      //LOG(INFO) << "FEDERATION: Response From Gossiper: " << MsgType << "  Num Char Read = " << numChar;
      break;
    }
    else if (MsgType == MSG_TYPE_HEARTBEAT)
    {
       numChar = write(gossiperSockfd,(void *) &ack,1);
       LOG(INFO) << "FEDERATION: Sent Heart Beat to Gossiper";
    }
    else if (MsgType == MSG_TYPE_DATA)
    {
       // Read the length of payload
       numChar = read(gossiperSockfd, &buf, 4);
       MsgLen = ntohl(buf);

       // when framework is not running Gossiper is sending zero info
       if(MsgLen <= 0 || numChar <= 0)
       {
	 LOG(WARNING) << "Looks like Framework is NOT running, so Gossiper sending BLANK info";
	 continue;
       }

       // For the first time table does not have anything
       if(fedOfferSuppressTable.size() <= 0 && notFirstTime == true)
       {
	 LOG(WARNING) << "Looks like Framework is NOT running, so Gossiper sending OLD info";
	 continue;
       }

       //LOG(INFO) << "FEDERATION: Gossiper Info";
       //LOG(INFO) << "FEDERATION: Gossiper Msg: " << buf  << " MsgLen: " << MsgLen;

       // Read the count of frameworks
       numChar = read(gossiperSockfd, &buf, 4);
       MsgCnt = ntohl(buf);
       //LOG(INFO) << "FEDERATION: Gossiper Msg: " << buf  << " MsgCnt: " << MsgCnt;

       char* gossiper_info = new char[MsgLen];
       numChar = read(gossiperSockfd, gossiper_info, MsgLen);

       pthread_mutex_lock(&mutexCondVarForFed);

       //LOG(INFO) << "FEDERATION: Gossiper table: Will Parse Now";
       bool isUpdated = ParseGossiperMessage(gossiper_info);

       delete [] gossiper_info;
      
       if(fedOfferSuppressTable.size() > 0)
       {
	  // Check if the table is modified or not
	  // depends on this send the signal to Fed Alloc module
	  if(isUpdated)
	  {
             LOG(INFO) << "FEDERATION: Now table update signal will be sent to Fed Alloc";	
             pthread_cond_signal(&condVarForFed);
          }
	  notFirstTime = true;
       }

       pthread_mutex_unlock(&mutexCondVarForFed);
    }
  } // while LOP ends here

}



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
    //LOG(INFO) << "FEDERATION: Accepted the Gossiper Connection Now";

    char clntName[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&gossiper_addr.sin_addr.s_addr,clntName,sizeof(clntName));

    unsigned int port = ntohs(gossiper_addr.sin_port);

    LOG(INFO) << "FEDERATION: Accepted Gossiper Machine Name : " << clntName << " and port # " << port;

    if(gossiperSockfd > 0)
      GetFrameworkInfoFromGossiper(gossiperSockfd);
    else
      LOG(ERROR) << "Error: Opening the gossiper machine socket";

    LOG(INFO) << "FEDERATION: Connection with Gossiper is LOST, will wait for new connection";
  }

}


static Anonymous* createFedCommunicator(const Parameters& parameters)
{
  auto obj = new FedCommunication();
  LOG(INFO) << "FEDERATION: Fed Comm Module is created and will be loaded";
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
