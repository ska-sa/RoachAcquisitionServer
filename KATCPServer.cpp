//System includes
#include <iostream>
#include <sstream>
#include <stdlib.h>

//Library includes
extern "C" {
#include <katpriv.h>
}

#include <boost/filesystem.hpp>

//Local includes
#include "KATCPServer.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"
#include "AVNUtilLibs/Sockets/InterruptibleBlockingSockets/InterruptibleBlockingTCPSocket.h"

using namespace std;

//Define static members of cKATCPServer:
cKATCPServer::cHDF5FileWriterCallbackHandler        cKATCPServer::m_oHDF5FileWriterCallBackHandler;
cKATCPServer::cKATCPClientCallbackHandler           cKATCPServer::m_oKATCPClientCallbackHandler;
struct katcp_dispatch                               *cKATCPServer::m_pKATCPDispatch;
boost::shared_ptr<cHDF5FileWriter>                  cKATCPServer::m_pFileWriter;
boost::shared_ptr<cRoachKATCPClient>                cKATCPServer::m_pRoachKATCPClient;
boost::shared_ptr<cStationControllerKATCPClient>    cKATCPServer::m_pStationControllerKATCPClient;
boost::scoped_ptr<boost::thread>                    cKATCPServer::m_pKATCPThread;
std::string                                         cKATCPServer::m_strListenInterface;
uint16_t                                            cKATCPServer::m_u16Port;
uint32_t                                            cKATCPServer::m_u32MaxClients;

cKATCPServer::cKATCPServer(const string &strInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
    startServer(strInterface, u16Port, u32MaxClients);
}

cKATCPServer::cKATCPServer()
{
}

cKATCPServer::~cKATCPServer()
{
    stopServer();

    if(m_pFileWriter.get())
        m_pFileWriter->deregisterCallbackHandler(&m_oHDF5FileWriterCallBackHandler);
}

void cKATCPServer::serverThreadFunction()
{
    // create a state handle
    m_pKATCPDispatch = startup_katcp();
    if(!m_pKATCPDispatch)
    {
        cout << "cKATCPServer::cKATCPServer(): Error allocating KATCP server state." << endl;
    }

    /* load up build and version information */
    //Get compile time
    stringstream oSSCompileTime;
    oSSCompileTime << string(__DATE__);
    oSSCompileTime << string(" ");
    oSSCompileTime << string(__TIME__);

    //Add a version number to KATCTP server
    add_version_katcp(m_pKATCPDispatch, "RoachAcquisitionServer", 0, "0.1", &oSSCompileTime.str()[0]);


    register_katcp(m_pKATCPDispatch, "?startRecording", "start data recording to HDF5", &cKATCPServer::startRecording_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?stopRecording",  "stop data recording to HDF5", &cKATCPServer::stopRecording_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?getRecordingInfo", "get info about current recording", &cKATCPServer::getRecordingInfo_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?getRecordingStatus", "is recording or not", &cKATCPServer::getRecordingStatus_KATCPCallback);

    //Make a server listening interface from hostname and port string
    stringstream oSSServer;
    oSSServer << m_strListenInterface;
    oSSServer << string(":");
    oSSServer << m_u16Port;

    if(run_multi_server_katcp(m_pKATCPDispatch, m_u32MaxClients, &oSSServer.str()[0], 0) < 0)
    {
        cout << "cKATCPServer::cKATCPServer(): Error starting KATCP server." << endl;
    }

    cout << "cKATCPServer::serverThreadFunction(): Exiting thread function." << endl;
}

void cKATCPServer::startServer(const string &strInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
    cout << "cKATCPServer::startServer() Starting KATCP server" << endl;

    //Store config parameters in members
    m_strListenInterface    = strInterface;
    m_u16Port               = u16Port;
    m_u32MaxClients         = u32MaxClients;

    //Launch KATCP server in a new thread
    m_pKATCPThread.reset(new boost::thread(&cKATCPServer::serverThreadFunction));
}

void cKATCPServer::stopServer()
{
    cout << "cKATCPServer::stopServer() Stopping KATCP server..." << endl;

    terminate_katcp(m_pKATCPDispatch, KATCP_EXIT_QUIT); //Stops server

    //Make socket connection to KATCP server to force its main loop to exit
    {
        cout << "Connect temporary socket to KATCP server to force it to evaluate shutdown request..." << endl;
        cInterruptibleBlockingTCPSocket oTempSocket(m_strListenInterface, m_u16Port);
    }

    //Now we can join the server thread
    m_pKATCPThread->join();
    m_pKATCPThread.reset();

    shutdown_katcp(m_pKATCPDispatch); //Cleans up memory of the dispatch struct

    cout << "cKATCPServer::stopServer() KATCP server stopped." << endl;
}

void cKATCPServer::setFileWriter(boost::shared_ptr<cHDF5FileWriter> pFileWriter)
{
    m_pFileWriter = pFileWriter;

    if(m_pFileWriter.get())
        m_pFileWriter->registerCallbackHandler(&m_oHDF5FileWriterCallBackHandler);
}

void cKATCPServer::setRoachKATCPClient(boost::shared_ptr<cRoachKATCPClient> pKATCPClient)
{
    m_pRoachKATCPClient = pKATCPClient;

    if(m_pRoachKATCPClient.get())
    {
        m_pRoachKATCPClient->registerCallbackHandler(&m_oKATCPClientCallbackHandler);
    }
}

void cKATCPServer::setStationControllerKATCPClient(boost::shared_ptr<cStationControllerKATCPClient> pKATCPClient)
{
    m_pStationControllerKATCPClient = pKATCPClient;

    if(m_pStationControllerKATCPClient.get())
        m_pStationControllerKATCPClient->registerCallbackHandler(&m_oKATCPClientCallbackHandler);
}

int32_t cKATCPServer::startRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    cout << "cKATCPServer::startRecording_KATCPCallback()" << endl;

    if(!m_pFileWriter.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "startRecording: No file recorder object set.");
        return KATCP_RESULT_FAIL;
    }

    if(i32ArgC > 4)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, "startRecording: Warning %i redundant argument(s) for stop recording, ignoring.", i32ArgC - 4);
    }
    string strFilePrefix("");
    int64_t i64StartTime_us = 0;
    int64_t i64Duration_us = 0;

    if(i32ArgC >= 2)
    {
        try
        {
            strFilePrefix = string(arg_string_katcp(pKATCPDispatch, 1));
        }
        catch(std::logic_error &oError)
        {
            cout << "cKATCPServer::startRecording_callback() Error unable to intepret filename prefix. Ignoring" << endl;
            cout << "Error was " << oError.what() << endl;
        }
    }

    if(i32ArgC >= 3)
        i64StartTime_us = strtoll(arg_string_katcp(pKATCPDispatch, 2), NULL, 10);

    if(i32ArgC >= 4)
        i64Duration_us = strtoll(arg_string_katcp(pKATCPDispatch, 3), NULL, 10);


    cout << "--------------------------------------------------------------" << endl;
    cout << "cKATCPServer::startRecording_callback() Got request to record:" << endl;
    cout << "File prefix = " << strFilePrefix << endl;
    cout << "Start time  = " << i64StartTime_us << " (" << AVN::stringFromTimestamp_full(i64StartTime_us) << ")" << endl;
    cout << "Duration    = " << i64Duration_us << " (" << AVN::stringFromTimeDuration(i64Duration_us) << ")" << endl;
    cout << "--------------------------------------------------------------" << endl;

    m_pFileWriter->startRecording(strFilePrefix, i64StartTime_us, i64Duration_us);

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::stopRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    cout << "cKATCPServer::stopRecording_KATCPCallback()" << endl;

    if(!m_pFileWriter.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "stopRecording: No file recorder object set.");
        return KATCP_RESULT_FAIL;
    }

    if(i32ArgC > 1)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, "stopRecording: Warning, %i redundant argument(s) for stop recording, ignoring.", i32ArgC - 1);
    }

    if(!m_pFileWriter->isRecordingEnabled())
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, "stopRecording: Warning, server is not currently recording");
        return KATCP_RESULT_FAIL;
    }

    m_pFileWriter->stopRecording();

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::getRecordingStatus_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, "#recordingStarted");
    }
    else
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, "#recordingStopped");
    }

    return KATCP_RESULT_OK;
}


int32_t cKATCPServer::getRecordingInfo_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        send_katcp( pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#recordingInfo");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%s", m_pFileWriter->getFilename().c_str());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%lli", m_pFileWriter->getRecordingStartTime_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%lli", m_pFileWriter->getRecordedDuration_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%lli", m_pFileWriter->getRecordingStopTime_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%lli", m_pFileWriter->getRecordingTimeLeft_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%llu", boost::filesystem::space(m_pFileWriter->getRecordingDirectory()).available);
    }
    else
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#recordingInfo");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "not recording");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%llu", boost::filesystem::space(m_pFileWriter->getRecordingDirectory()).available);
    }

    return KATCP_RESULT_OK;
}

void cKATCPServer::cHDF5FileWriterCallbackHandler::recordingStarted_callback()
{
    struct katcp_shared *s;
    struct katcp_dispatch *dx;

    s = m_pKATCPDispatch->d_shared;

    //Send to each client
    for(uint32_t ui = 0; ui < s->s_used; ui++)
    {
        dx = s->s_clients[ui];
        send_katcp(dx, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, "#recordingStarted");
        write_katcp(dx); //Required to flush
    }

    cout << "cKATCPServer::cHDF5FileWriterNotifier::recordingStarted() Got notification, sent KATCP notification message to all clients." << endl;
}

void cKATCPServer::cHDF5FileWriterCallbackHandler::recordingStopped_callback()
{
    struct katcp_shared *s;
    struct katcp_dispatch *dx;

    s = m_pKATCPDispatch->d_shared;

    //Send to each client
    for(uint32_t ui = 0; ui < s->s_used; ui++)
    {
        dx = s->s_clients[ui];
        send_katcp( dx, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, "#recordingStopped");
        write_katcp(dx); //Required to flush
    }

    cout << "cKATCPServer::cHDF5FileWriterNotifier::recordingStopped() Got notification, sent broadcast KATCP message" << endl;
}

void cKATCPServer::cKATCPClientCallbackHandler::connected_callback(bool bConnected)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#connectedToStationController");
    if(bConnected)
    {
        append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "1");
    }
    else
    {
        append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "0");
    }
}

void cKATCPServer::cKATCPClientCallbackHandler::startRecording_callback(const string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void cKATCPServer::cKATCPClientCallbackHandler::stopRecording_callback()
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void  cKATCPServer::cKATCPClientCallbackHandler::requestedAntennaAzEl_callback(int64_t i64Timestamp_us, double dAzimuth_deg, double dElevation_deg)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#requestedAntennaAzEl");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dAzimuth_deg);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dElevation_deg);
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaAzEl_callback(int64_t i64Timestamp_us, double dAzimuth_deg, double dElevation_deg)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#actualAntennaAzEl");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dAzimuth_deg);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dElevation_deg);
}

void cKATCPServer::cKATCPClientCallbackHandler::actualSourceOffsetAzEl_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg, double dElevationOffset_deg)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#actualSourceOffsetAzEl");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dAzimuthOffset_deg);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dElevationOffset_deg);
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaRADec_callback(int64_t i64Timestamp_us, double dRighAscension_deg, double dDeclination_deg)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#actualAntennaRADec");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dRighAscension_deg);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dDeclination_deg);
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaStatus_callback(int64_t i64Timestamp_us, int32_t i32AntennaStatus, const string &strStatus)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#antennaStatus");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%i",i32AntennaStatus);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%s", strStatus.c_str());
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorques_callback(int64_t i64Timestamp_us, double dAz0_Nm, double dAz1_Nm, double dEl0_Nm, double dEl1_Nm)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#motorTorques");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dAz0_Nm);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dAz1_Nm);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dEl0_Nm);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f", dEl1_Nm);
}

void cKATCPServer::cKATCPClientCallbackHandler::appliedPointingModel_callback(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#appliedPointingModel");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%s", strModelName.c_str());
    for(uint32_t i = 0; i < (uint32_t)vdPointingModelParams.size() -1; i++)
    {
        append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", vdPointingModelParams[i]);
    }
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",vdPointingModelParams[vdPointingModelParams.size() -1]);
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#noiseDiodeSoftwareState");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%i",i32NoiseDiodeState);
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeSource_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSource, const string &strNoiseSource)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#noiseDiodeSource");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%i",i32NoiseDiodeSource);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%s",strNoiseSource.c_str());
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#noideDiodeCurrent");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dNoiseDiodeCurrent_A);
}

void cKATCPServer::cKATCPClientCallbackHandler::sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#sourceSelection");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%s", strSourceName.c_str());
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dRighAscension_deg);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dDeclination_deg);
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyRF_callback(int64_t i64Timestamp_us, double dFreqencyRF_MHz)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#frequencyRF");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dFreqencyRF_MHz);
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLOs_callback(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, double dFrequencyLO2_MHz)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#frequencyLOs");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f",dFrequencyLO1_MHz);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dFrequencyLO2_MHz);
}

void cKATCPServer::cKATCPClientCallbackHandler::bandwidthIF_callback(int64_t i64Timestamp_us, double dBandwidthIF_MHz)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#bandwidthIF");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dBandwidthIF_MHz);
}

void cKATCPServer::cKATCPClientCallbackHandler::accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#accumulationLength");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%u",u32NFrames);
}

void cKATCPServer::cKATCPClientCallbackHandler::narrowBandChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#narrowBandChannelSelect");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%u",u32ChannelNo);
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyFs_callback(double dFrequencyFs_MHz)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#frequencyFs");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dFrequencyFs_MHz);
}

void cKATCPServer::cKATCPClientCallbackHandler::sizeOfFFTs_callback(uint32_t u32CoarseSize_nSamp, uint32_t u32FineSize_nSamp)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#sizeOfFFTs");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%u", u32CoarseSize_nSamp);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%u",u32FineSize_nSamp);
}

void cKATCPServer::cKATCPClientCallbackHandler::coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#coarseFFTShift");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%u", u32ShiftMask);
}

void cKATCPServer::cKATCPClientCallbackHandler::adcAttenuation_callback(int64_t i64Timestamp_us, double dAttenuationChan0_dB, double dAttenuationChan1_dB)
{
    send_katcp( m_pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#adcAttenuation");
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%lli", i64Timestamp_us);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_STRING, "%f", dAttenuationChan0_dB);
    append_args_katcp(m_pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%f",dAttenuationChan1_dB);
}
