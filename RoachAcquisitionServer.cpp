
//System includes:

//Library includes:
#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/make_shared.hpp>
#endif

//Local includes:
#include "RoachAcquisitionServer.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"

cRoachAcquisitionServer::cRoachAcquisitionServer(const string &strLocalInterface, uint16_t u16LocalPort, const string &strRoachTGBEAddress, uint16_t u16RoachTGBEPort,
                                                 const string strClientInterface, uint16_t u16ClientDataPort) :
    m_strRoachTGBEAddress(strRoachTGBEAddress),
    m_u16RoachTGBEPort(u16RoachTGBEPort),
    m_strLocalInterface(strLocalInterface),
    m_u16LocalPort(u16LocalPort),
    m_strClientInterface(strClientInterface),
    m_u16ClientDataPort(u16ClientDataPort)
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
    m_pUDPReceiver         = boost::make_shared<cUDPReceiver>(m_strLocalInterface, m_u16LocalPort, m_strRoachTGBEAddress, m_u16RoachTGBEPort);

    //Data output handlers
    m_pTCPForwardingServer = boost::make_shared<cTCPForwardingServer>(m_strClientInterface, m_u16ClientDataPort, 5);
    m_pHDF5FileWriter      = boost::make_shared<cHDF5FileWriter>(std::string("/home/avnuser/Data/RoachAquisition"));

    //Register the output handlers to get data from the input handler
    m_pUDPReceiver->registerDataCallbackHandler(m_pTCPForwardingServer);
    m_pUDPReceiver->registerDataCallbackHandler(m_pHDF5FileWriter);

    m_pUDPReceiver->startCallbackOffloading();
    m_pUDPReceiver->startReceiving();
}

void cRoachAcquisitionServer::shutdown()
{   
    //Delete all components...
    m_pHDF5FileWriter->stopRecording();
    m_pHDF5FileWriter->waitForFileClosed(); //This class requires callback pushes to change state so block here until it has reach an idle state

    m_pUDPReceiver.reset();
    m_pTCPForwardingServer.reset();
    m_pKATCPServer.reset();
    m_pHDF5FileWriter.reset();

    m_pRoachKATCPClient.reset();
    m_pStationControllerKATCPClient.reset();

    cout << std::flush;

    cout << "cRoachAcquisitionServer::shutdown() all components deleted." << endl;
}

void cRoachAcquisitionServer::startKATCPServer(std::string strInterface, uint16_t u16Port)
{
    if(m_pKATCPServer.get())
    {
        cout << "cRoachAcquisitionServer::startKATCPServer(): KATCP server already running. Ignoring." << endl;
        return;
    }

    //Start the KATCP control and monitoring server
    m_pKATCPServer = boost::make_shared<cKATCPServer>(strInterface, u16Port);

    //Connect file writer
    m_pKATCPServer->setFileWriter(m_pHDF5FileWriter);

    //If available connect to KATCP clients
    if(m_pRoachKATCPClient.get())
    {
        m_pKATCPServer->setRoachKATCPClient(m_pRoachKATCPClient);
    }

    if(m_pStationControllerKATCPClient.get())
    {
        m_pKATCPServer->setStationControllerKATCPClient(m_pStationControllerKATCPClient);
    }
}

void cRoachAcquisitionServer::stopKATCPServer()
{
    m_pKATCPServer.reset();
}

void cRoachAcquisitionServer::startRoachKATCPClient(std::string strServerAddress, uint16_t u16Port)
{
    if(m_pRoachKATCPClient.get())
    {
        cout << "cRoachAcquisitionServer::startKATCPClient_roach(): KATCP client already running. Ignoring." << endl;
        return;
    }

    //Start the KATCP client connection to Roach
    m_pRoachKATCPClient = boost::make_shared<cRoachKATCPClient>(strServerAddress, u16Port);

    //Connection to server if available
    if(m_pKATCPServer.get())
    {
        m_pKATCPServer->setRoachKATCPClient(m_pRoachKATCPClient);
    }
}

void cRoachAcquisitionServer::stopRoachKATCPClient()
{
    //Disconnect from server if available
    if(m_pKATCPServer.get())
    {
        m_pKATCPServer->setRoachKATCPClient(boost::make_shared<cRoachKATCPClient>());
    }

    m_pRoachKATCPClient.reset();
}

void cRoachAcquisitionServer::startStationControllerKATCPClient(std::string strServerAddress, uint16_t u16Port)
{
    if(m_pStationControllerKATCPClient.get())
    {
        cout << "cRoachAcquisitionServer::startKATCPClient_stationController(): KATCP client already running. Ignoring." << endl;
        return;
    }

    //Start the KATCP client connection to Roach
    m_pStationControllerKATCPClient = boost::make_shared<cStationControllerKATCPClient>(strServerAddress, u16Port);

    //Connection to server if available
    if(m_pKATCPServer.get())
    {
        m_pKATCPServer->setStationControllerKATCPClient(m_pStationControllerKATCPClient);
    }
}

void cRoachAcquisitionServer::stopStationControllerKATCPClient()
{
    //Disconnect from server if available
    if(m_pKATCPServer.get())
    {
        m_pKATCPServer->setStationControllerKATCPClient(boost::make_shared<cStationControllerKATCPClient>());
    }

    m_pStationControllerKATCPClient.reset();
}
