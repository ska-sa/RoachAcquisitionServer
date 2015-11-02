
//System includes

//Library include

//Local includes
#include "TCPForwardingServer.h"

cTCPForwardingServer::cTCPForwardingServer(const std::string &strInterface, uint16_t u16Port, uint32_t u32MaxConnections) : cTCPServer(strInterface, u16Port, u32MaxConnections)
{
}

cTCPForwardingServer::~cTCPForwardingServer()
{
}

void cTCPForwardingServer::offloadData_callback(char* cpData, uint32_t u32Size_B)
{
    writeData(cpData, u32Size_B);
}
