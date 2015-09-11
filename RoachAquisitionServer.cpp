
//System includes:

//Library includes:
#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/make_shared.hpp>
#endif

//Local includes:
#include "RoachAquisitionServer.h"

cRoachAquisitionServer::cRoachAquisitionServer(const string &strRoachTGBEAddress, uint16_t u16RoachTGBEPort) :
    m_strRoachTGBEAddress(strRoachTGBEAddress),
    m_u16RoachTGBEPort(u16RoachTGBEPort)
{
    start();
}

cRoachAquisitionServer::~cRoachAquisitionServer()
{
    m_pUDPReceiver.reset();
    m_pTCPForwardingServer.reset();

    cout << "All members deleted." << endl;
}

void cRoachAquisitionServer::start()
{
    m_pTCPForwardingServer = boost::make_shared<cTCPForwardingServer>("0.0.0.0", 60001, 2);
    m_pUDPReceiver.reset(new cUDPReceiver(m_strRoachTGBEAddress, m_u16RoachTGBEPort));

    m_pUDPReceiver->registerCallbackHandler(m_pTCPForwardingServer);

    m_pUDPReceiver->startCallbackOffloading();
    m_pUDPReceiver->startReceiving();
}

void cRoachAquisitionServer::shutdown()
{
    m_pUDPReceiver.reset();
    m_pTCPForwardingServer.reset();
}
