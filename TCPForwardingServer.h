#ifndef TCP_FORWARDING_SERVER_H
#define TCP_FORWARDING_SERVER_H

//System includes

//Library include:

//Local includes
#include "AVNAppLibs/SocketStreamers/TCPServer/TCPServer.h"
#include "AVNAppLibs/SocketStreamers/UDPReceiver/UDPReceiver.h"

class cTCPForwardingServer : public cTCPServer, public cUDPReceiver::cUDPReceiverCallbackInterface
{
public:
    cTCPForwardingServer(const std::string &strInterface = std::string("0.0.0.0"), uint16_t usPort = 60001, uint32_t u32MaxConnections = 0);
    ~cTCPForwardingServer();

    bool offloadData_callback(char* cpData, uint32_t u32Size_B);
};

#endif //TCP_FORWARDING_SERVER_H
