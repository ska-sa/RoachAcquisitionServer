
//System includes:

//Library includes:
#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/make_shared.hpp>
#endif

//Local includes:
#include "RoachAcquisitionServer.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"

cRoachAcquisitionServer::cRoachAcquisitionServer(const string &strLocalInterface, uint16_t u16LocalPort, const string &strRoachTGBEAddress, uint16_t u16RoachTGBEPort,
                                                 const string &strClientInterface, uint16_t u16ClientDataPort,
                                                 const string &strRecordingDir, uint32_t u32MaxFileSize_MB) :
    m_strRoachTGBEAddress(strRoachTGBEAddress),
    m_u16RoachTGBEPort(u16RoachTGBEPort),
    m_strLocalInterface(strLocalInterface),
    m_u16LocalPort(u16LocalPort),
    m_strClientInterface(strClientInterface),
    m_u16ClientDataPort(u16ClientDataPort),
    m_strRecordingDir(strRecordingDir),
    m_u32MaxFileSize_MB(u32MaxFileSize_MB)
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
    m_pHDF5FileWriter      = boost::make_shared<cHDF5FileWriter>(std::string("/home/avnuser/Data/RoachAquisition"), m_u32MaxFileSize_MB);

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
    m_pRoachKATCPClient.reset();
    m_pStationControllerKATCPClient.reset();
    m_pKATCPServer.reset();
    m_pHDF5FileWriter.reset();

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
        cout << "cRoachAcquisitionServer::startRoachKATCPClient(): KATCP client already running. Ignoring." << endl;
        return;
    }

    //Start the KATCP client connection to Roach
    cout << "cRoachAcquisitionServer::startRoachKATCPClient(): Starting KATCP client for ROACH TCPBorph." << endl;
    m_pRoachKATCPClient = boost::make_shared<cRoachKATCPClient>();
    m_pRoachKATCPClient->connect(strServerAddress, u16Port);

    //The cKATCPClientBase::CallInterface implementation needs to be passed as base pointer type.
    //Because the HDF5FileWriter derives 2 variations of cKATCPClientBase::CallInterface derivations,
    //we need to work back through the inheritance hierachy with dynamic casts to unabigouisly get the pointer
    //as a base type.
    boost::shared_ptr<cRoachKATCPClient::cCallbackInterface> pRoachKATCPTClientCallbackInterface = boost::dynamic_pointer_cast<cRoachKATCPClient::cCallbackInterface>(m_pHDF5FileWriter);
    boost::shared_ptr<cKATCPClientBase::cCallbackInterface> pKATCPClientBaseCallbackInterface = boost::dynamic_pointer_cast<cKATCPClientBase::cCallbackInterface>(pRoachKATCPTClientCallbackInterface);
    m_pRoachKATCPClient->registerCallbackHandler(pKATCPClientBaseCallbackInterface);

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
        cout << "cRoachAcquisitionServer::startStationControllerKATCPClient(): KATCP client already running. Ignoring." << endl;
        return;
    }

    //Start the KATCP client connection to Roach
    cout << "cRoachAcquisitionServer::startStationControllerKATCPClient(): Starting KATCP client for station controller." << endl;
    m_pStationControllerKATCPClient = boost::make_shared<cStationControllerKATCPClient>();
    m_pStationControllerKATCPClient->connect(strServerAddress, u16Port);

    //The cKATCPClientBase::CallInterface implementation needs to be passed as base pointer type.
    //Because the HDF5FileWriter derives 2 variations of cKATCPClientBase::CallInterface derivations,
    //we need to work back through the inheritance hierachy with dynamic casts to unabigouisly get the pointer
    //as a base type.
    boost::shared_ptr<cStationControllerKATCPClient::cCallbackInterface> pStationControllerKATCPTClientCallbackInterface
            = boost::dynamic_pointer_cast<cStationControllerKATCPClient::cCallbackInterface>(m_pHDF5FileWriter);
    boost::shared_ptr<cKATCPClientBase::cCallbackInterface> pKATCPClientBaseCallbackInterface
            = boost::dynamic_pointer_cast<cKATCPClientBase::cCallbackInterface>(pStationControllerKATCPTClientCallbackInterface);
    m_pStationControllerKATCPClient->registerCallbackHandler(pKATCPClientBaseCallbackInterface);

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
