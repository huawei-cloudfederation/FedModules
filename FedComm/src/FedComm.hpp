#ifndef __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION__
#define __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION__

#include <mesos/module/anonymous.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../Fed_Common.hpp"


typedef enum
{
  MSG_TYPE_ACK,
  MSG_TYPE_FW_SUPP_INFO,

} MsgType_E;


using mesos::modules::Anonymous;


pthread_mutex_t mutex_fed_offer_suppress_table = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var_filter = PTHREAD_COND_INITIALIZER;

std::map <string, Suppress_T> fed_offer_suppress_table;


void* PollGossiper(void*);
int ConnectToGossiper();


class FedCommunication : public Anonymous
{
  public:
    FedCommunication();
    virtual ~FedCommunication();
};

#endif // __MESOS_FEDERATION_ANONYMOUS_COMMUNICATION___

