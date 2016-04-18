#ifndef __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION__
#define __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION__

//#include "../../Fed_Common.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <mesos/module/anonymous.hpp>
#include "../../Fed_Common.hpp"

using mesos::modules::Anonymous;

// Types of Messages Sent over TCP/IP connection
// Between Gossiper and Fed Comm module
typedef enum
{
  MSG_TYPE_ACK,
  MSG_TYPE_DATA,
  MSG_TYPE_HEARTBEAT

} MsgType_E;


pthread_cond_t condVarForFed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexCondVarForFed = PTHREAD_MUTEX_INITIALIZER;

std::map <string, Suppress_T> fedOfferSuppressTable;
pthread_mutex_t mutexFedOfferSuppressTable = PTHREAD_MUTEX_INITIALIZER;

// Forward declaration for a thread function
void* PollFedGossiper(void*);


// Annonymous mesos module sits b/w Gossiper and Fed Allocator
class FedCommunication : public Anonymous
{
public:
  FedCommunication();
  virtual ~FedCommunication();
  void OpenServerSocket(); // Which will open the socket at server end

private:
  int fedCommSockfd, portno;
  struct sockaddr_in fedComm_addr;
  pthread_t threadId;
};

#endif // __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION___

