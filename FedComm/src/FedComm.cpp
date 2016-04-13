#include "FedComm.hpp"
#include "../../FedUtil/FedUtil.hpp"

#include <glog/logging.h>


// Constructor
FedCommunication :: FedCommunication()
{
  pthread_t threadId;
  pthread_create(&threadId, NULL, PollGossiper, NULL);
  //pthread_create(&threadId, NULL, PollGossiper, NULL);
}


// Destructor
FedCommunication :: ~FedCommunication()
{
  LOG(INFO) << "FedCommunication Destructor";
}


// Parser for gossiper messag
void ParseGossiperMessage(char* gossiper_info)
{
  stringstream str(gossiper_info);
  string token;
  stringstream json;

  json << "{";

  while (str >> token)
  {
    int sep = token.find(':');
    string fId(token, 0, sep);

    if (fedOfferSuppressTable.find(fId) != fedOfferSuppressTable.end())
    {
      fedOfferSuppressTable[fId].supByFederationFlag = (token[sep+1] == '1');
    }

    json << fId <<":" << (token[sep+1] == '1') <<" ";
  }

  json << "}";
  LOG(INFO) << json.str();
}


int ConnectToGossiper()
{
  int sockfd;
  struct sockaddr_in serv_addr;

  Config cfg;
  ReadConfig(cfg);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    LOG(ERROR) << "ERROR opening socket";
    return 0;
  }
  else
    LOG(INFO) << "Opened socket";

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  inet_aton((const char*)cfg.gossiper_ip.c_str(), (struct in_addr*)(&(serv_addr.sin_addr)));
  serv_addr.sin_port = htons(cfg.gossiper_port);

  while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    LOG(ERROR) << "ERROR connecting to " << cfg.gossiper_ip.c_str() <<":" << cfg.gossiper_port;
    sleep(1); // sllep for 1 sec before trying to connect again
  }

  LOG(INFO) << "Connected to " << cfg.gossiper_ip.c_str() <<":" << cfg.gossiper_port;
 
  return sockfd;
}


// Thread function
void* PollGossiper(void* arg)
{
  int n;
  const char* HeartBeat = "H";
  char MsgType;
  unsigned int buf;
  unsigned long MsgLen;
  unsigned long MsgCnt;

  int sockfd = ConnectToGossiper();
  if (!sockfd)
    return 0;

  while (1)
  {
    // HeartBeat
    n = write(sockfd, HeartBeat, strlen(HeartBeat));
    if (n < 0)
    {
      LOG(ERROR) << "ERROR writing to socket";
      close(sockfd);
      sockfd = ConnectToGossiper();
      if (!sockfd)
        return 0;

    }
    else
      LOG(INFO) << "HeartBeat sent";

    /*unsigned char c;
    n = read(sockfd, &c, 1);
    putchar(c);
    fflush(stdout);
    continue;*/

    // Read message type
    n = read(sockfd, &MsgType, 1);

    // It's an ACK
    if (MsgType == MSG_TYPE_ACK)
    {
      LOG(INFO) << "Gossiper ACK";
    }
    // we've got an update from Gossiper
    //else if (MsgType == MSG_TYPE_FW_SUPP_INFO)
    else
    {
      LOG(INFO) << "Gossiper Info";
      // Read the length of payload
      n = read(sockfd, &buf, 4);
      MsgLen = ntohl(buf);
      LOG(INFO) << "Gossiper MsgLen: " << MsgLen;

      // Read the count of frameworks
      n = read(sockfd, &buf, 4);
      MsgCnt = ntohl(buf);
      LOG(INFO) << "Gossiper MsgCnt: " << MsgCnt;

      char* gossiper_info = new char[MsgLen];
      n = read(sockfd, gossiper_info, MsgLen);

      pthread_mutex_lock(&mutexCondVarForFed);

      LOG(INFO) << "Gossiper table: ";
      ParseGossiperMessage(gossiper_info);

      delete [] gossiper_info;
      
      pthread_cond_signal(&condVarForFed);
      pthread_mutex_unlock(&mutexCondVarForFed);
    }
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
  "Huawei Mesos Federation Project",
  "parushuram.k@huawei.com",
  "Mesos Federation Communication Module.",
  NULL,
  createFedCommunicator);
