
//System includes:

//Library includes:
#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/make_shared.hpp>
#endif

//Local includes:
#include "RoachAcquisitionServer.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"

cRoachAcquisitionServer::cRoachAcquisitionServer(const string &strLocalInterface, uint16_t u16LocalPort, const string &strRoachTGBEAddress, uint16_t u16RoachTGBEPort) :
    m_strRoachTGBEAddress(strRoachTGBEAddress),
    m_u16RoachTGBEPort(u16RoachTGBEPort),
    m_strLocalInterface(strLocalInterface),
    m_u16LocalPort(u16LocalPort)
{
    start();
}

cRoachAcquisitionServer::~cRoachAcquisitionServer()
{
    shutdown();

    cout << "cRoachAcquisitionServer::~cRoachAcquisitionServer(): All memory cleaned up." << endl;
}

void cRoachAcquisitionServer::start()
{
    //Data input handlers
    m_pUDPReceiver.reset(new cUDPReceiver(m_strLocalInterface, m_u16LocalPort, m_strRoachTGBEAddress, m_u16RoachTGBEPort));

    //Data output handlers
    m_pTCPForwardingServer = boost::make_shared<cTCPForwardingServer>("0.0.0.0", 60001, 2);
    m_pHDF5FileWriter      = boost::make_shared<cHDF5FileWriter>();

    //Register the output handlers to get data from the input handler
    m_pUDPReceiver->registerCallbackHandler(m_pTCPForwardingServer);
    m_pUDPReceiver->registerCallbackHandler(m_pHDF5FileWriter);

    m_pUDPReceiver->startCallbackOffloading();
    m_pUDPReceiver->startReceiving();

    //Testing
    m_pHDF5FileWriter->startRecording(std::string("/usr2/charles/ctong/HDF5Output"), std::string("HDF5_Test_"), AVN::getTimeNow_us() + 3000000LL, 500000000LL);
}

void cRoachAcquisitionServer::shutdown()
{
    m_pUDPReceiver->deregisterCallbackHandler(m_pTCPForwardingServer);
    m_pUDPReceiver.reset();
    m_pTCPForwardingServer.reset();
}
