#ifndef __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION__
#define __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION__

#include <mesos/module/anonymous.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../Fed_Common.hpp"

using mesos::modules::Anonymous;

typedef enum
{
  MSG_TYPE_ACK,
  MSG_TYPE_DATA,
  MSG_TYPE_HEARTBEAT
} MsgType_E;


pthread_mutex_t mutexFedOfferSuppressTable = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condVarForFed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexCondVarForFed = PTHREAD_MUTEX_INITIALIZER;

std::map <string, Suppress_T> fedOfferSuppressTable;

void* PollFedGossiper(void*);


class FedCommunication : public Anonymous
{
public:
  FedCommunication();
  virtual ~FedCommunication();
  void OpenServerSocket();

private:
  int fedCommSockfd, portno;
  struct sockaddr_in fedComm_addr;
  pthread_t threadId;
};

#endif // __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION___

