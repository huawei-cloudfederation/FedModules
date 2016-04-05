#include "FedComm.hpp"
#include "../../FedUtil.hpp"

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

    while (str >> token)
    {
        int sep = token.find(':');
        string fId(token, 0, sep);
        fed_offer_suppress_table[fId].federation = (token[sep+1] == '1');
    }
}

/*
Thread function
*/
void* PollGossiper(void* arg)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;

    const char* HeartBeat = "H";
    unsigned char MsgType;
	char buf[4];
    unsigned long MsgLen;
    unsigned long MsgCnt;

    Config cfg;
    ReadConfig(cfg);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cout << "========== HUAWEI - " << "ERROR opening socket" <<endl;
    
    cout << "========== HUAWEI - " << "opened socket" <<endl;

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    cout << "**************************\n";
    inet_aton((const char*)cfg.gossiper_ip.c_str(), (struct in_addr*)(&(serv_addr.sin_addr)));
    serv_addr.sin_port = htons(cfg.gossiper_port);

    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        cout << "========== HUAWEI - ERROR connecting" <<endl;
    
    cout << "========== HUAWEI - connected to " << cfg.gossiper_ip.c_str() <<":" << cfg.gossiper_port << endl;

    while (1)
    {
        char c;
        sleep (1);

        // HeartBeat
        n = write(sockfd, HeartBeat, strlen(HeartBeat));
        if (n < 0) 
            cout << "========== HUAWEI - " << "ERROR writing to socket" <<endl;
			return 0;
        
        //cout << "========== HUAWEI - " << "HeartBeat sent" <<endl;

        n = read(sockfd, &c, 1);
        putchar(c);
		continue;

        // Read message type
        n = read(sockfd, &MsgType, 1);

        // It's an ACK
        if (MsgType == MSG_TYPE_ACK)
        {
            cout << "========== HUAWEI - Gossiper ACK" << endl;
        }
        // we've got an update from Gossiper
        //else if (MsgType == MSG_TYPE_FW_SUPP_INFO)
        {
            cout << "=========== ========== ========= HUAWEI - Gossiper Info" << endl;
            // Read the length of payload
            n = read(sockfd, buf, 4);
			MsgLen = atoi(buf);
			MsgLen = ntohl(MsgLen);
            cout << "========== HUAWEI - Gossiper MsgLen : " << MsgLen << endl;

            // Read the count of frameworks
            n = read(sockfd, buf, 4);
			MsgCnt = atoi(buf);
			MsgCnt = ntohl(MsgCnt);
            cout << "========== HUAWEI - Gossiper MsgCnt: " << MsgCnt << endl;

            char* gossiper_info = new char[MsgLen];
            n = read(sockfd, gossiper_info, MsgLen);

			pthread_mutex_lock(&mutex_fed_offer_suppress_table);

            ParseGossiperMessage(gossiper_info);

            cout << "========== HUAWEI - Gossiper table: " << endl;
            for (map<string, Suppress_T>::iterator it = fed_offer_suppress_table.begin(); it!=fed_offer_suppress_table.end(); ++it)
            {
                string id = it->first;
                bool suppress_fed = it->second.federation;
                bool suppress_frm = it->second.framework;

                cout << id <<"  " << suppress_fed <<"  " << suppress_frm << endl; 
            }
            delete [] gossiper_info;

            pthread_cond_signal(&cond_var_filter);
			pthread_mutex_unlock(&mutex_fed_offer_suppress_table);

            // Set cond var
				;
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

