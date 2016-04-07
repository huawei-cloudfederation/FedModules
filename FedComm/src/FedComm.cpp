#include "FedComm.hpp"
#include "../../FedUtil/FedUtil.hpp"


// Constructor
FedCommunication :: FedCommunication()
{
    cout << "========== HUAWEI - FedCommunication Constructor" << endl;

    pthread_t threadId_poll_gosipper;
    pthread_create(&threadId_poll_gosipper, NULL, PollGossiper, NULL);
}

// Destructor
FedCommunication :: ~FedCommunication()
{
    cout << "========== HUAWEI - FedCommunication Destructor" << endl;
}

/*
Parser for gossiper message
*/
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

        if (fed_offer_suppress_table.find(fId) != fed_offer_suppress_table.end())
        {
            fed_offer_suppress_table[fId].federation = (token[sep+1] == '1');
        }

        json << fId <<":" << (token[sep+1] == '1') <<" ";
    }

    json << "}";
    cout << json.str() << endl;;
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
        cout << "========== HUAWEI - " << "ERROR opening socket" <<endl;
        return 0;
    }
    else
        cout << "========== HUAWEI - " << "opened socket" <<endl;

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_aton((const char*)cfg.gossiper_ip.c_str(), (struct in_addr*)(&(serv_addr.sin_addr)));
    serv_addr.sin_port = htons(cfg.gossiper_port);

    while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "========== HUAWEI - ERROR connecting to " << cfg.gossiper_ip.c_str() <<":" << cfg.gossiper_port <<endl;
        sleep(1); // sllep for 1 sec before trying to connect again
    }

    cout << "========== HUAWEI - connected to " << cfg.gossiper_ip.c_str() <<":" << cfg.gossiper_port << endl;
 
    return sockfd;
}

/*
Thread function
*/
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
            cout << "========== HUAWEI - " << "ERROR writing to socket" <<endl;
            sockfd = ConnectToGossiper();
            if (!sockfd)
                return 0;

        }
        else
            cout << "========== HUAWEI - " << "HeartBeat sent" <<endl;

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
            cout << "========== HUAWEI - Gossiper ACK" << endl;
        }
        // we've got an update from Gossiper
        //else if (MsgType == MSG_TYPE_FW_SUPP_INFO)
        else
        {
            cout << "=========== ========== ========= HUAWEI - Gossiper Info" << endl;
            // Read the length of payload
            n = read(sockfd, &buf, 4);
            MsgLen = ntohl(buf);
            cout << "========== HUAWEI - Gossiper MsgLen : " << MsgLen << endl;

            // Read the count of frameworks
            n = read(sockfd, &buf, 4);
            MsgCnt = ntohl(buf);
            cout << "========== HUAWEI - Gossiper MsgCnt: " << MsgCnt << endl;

            char* gossiper_info = new char[MsgLen];
            n = read(sockfd, gossiper_info, MsgLen);

            pthread_mutex_lock(&mutex_fed_offer_suppress_table);

            cout << "========== HUAWEI - Gossiper table: " << endl;
            ParseGossiperMessage(gossiper_info);
            cout << endl;

            delete [] gossiper_info;
            
            pthread_cond_signal(&cond_var_filter);
            pthread_mutex_unlock(&mutex_fed_offer_suppress_table);
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
    "Huawei Mesos Federation",
    "parushuram.k@huawei.com",
    "Mesos Federation Communication Module.",
    NULL,
    createFedCommunicator);
