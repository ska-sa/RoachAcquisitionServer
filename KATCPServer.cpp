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

    //Deregister the all back handlers in this class from their resepctive callers
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

    //Declare sensors
    //Station controller
    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, "stationControllerConnected", "Is the RoachAcquisitionServer connected to the StationController's KATCP server", "none",
                                  &getIsStationControllerKATCPConnected, NULL, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "requestAntennaAz", "requested antenna azimuth after the pointing model", "degrees",
                                 &getRequestedAntennaAz, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "requestAntennaEl", "requested antenna elevation after the pointing model", "degrees",
                                 &getRequestedAntennaEl, NULL, NULL, 0.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "actualAntennaAz", "actual antenna azimuth after the pointing model", "degrees",
                                 &getActualAntennaAz, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "actualAntennaEl", "actual antenna elevation after the pointing model", "degrees",
                                 &getActualAntennaEl, NULL, NULL, 0.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "actualSourceOffsetAz", "actual azimuth offset from the source", "degrees",
                                 &getActualSourceOffsetAz, NULL, NULL, -90.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "actualSourceOffsetEl", "actual elevation offset from the source", "degrees",
                                 &getActualSourceOffsetEl, NULL, NULL, -90.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "actualAntennaRA", "actual antenna right ascension", "degrees",
                                 &getActualAntennaRA, NULL, NULL, -90.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "actualAntennaDec", "actual antenna declination", "degrees",
                                 &getActualAntennaDec, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "motorTorqueAzMaster", "torque of master azimuth motor", "mNm",
                                 &getMotorTorqueAzMaster, NULL, NULL, -3600.0, 3600.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "motorTorqueAzSlave", "torque of slave azimuth motor", "mNm",
                                 &getMotorTorqueAzSlave, NULL, NULL, -3600.0, 3600.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "motorTorqueElMaster", "torque of master elevation motor", "mNm",
                                 &getMotorTorqueElMaster, NULL, NULL, -3600.0, 3600.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "motorTorqueElSlave", "torque of slave elevation motor", "mNm",
                                 &getMotorTorqueElSlave, NULL, NULL, -3600.0, 3600.0, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "noiseDiodeSoftwareState", "the state of the noise diode (on/off) when under FieldSystem control", "none",
                                  &getNoiseDiodeSoftwareState, NULL, NULL, -1, 1, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "noiseDiodeCurrent", "the current draw through the noise diode(s)", "A",
                                 &getMotorTorqueElSlave, NULL, NULL, 0.0, 5.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "frequencyRFChan0", "the centre frequency of RF mapped to final IF output channel 0", "MHz",
                                 &getFrequencyRFChan0, NULL, NULL, 4800.0, 7000.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "frequencyRFChan0", "the centre frequency of RF mapped to final IF output channel 1", "MHz",
                                 &getFrequencyRFChan1, NULL, NULL, 4800.0, 7000.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "frequencyLO0Chan0", "the frequency for the first LO in RF input channel 0", "MHz",
                                 &getFrequencyLO0Chan0, NULL, NULL, 0, 7000.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "frequencyLO0Chan0", "the frequency for the first LO in RF input channel 1", "MHz",
                                 &getFrequencyLO1, NULL, NULL, 0, 7000.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "receiverBandwidthChan0", "the bandwidth available to the final IF output channel 0", "MHz",
                                 &getReceiverBandwidthChan0, NULL, NULL, 0, 7000.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "receiverBandwidthChan1", "the bandwidth available to the final IF output channel 1", "MHz",
                                 &getReceiverBandwidthChan1, NULL, NULL, 0, 7000.0, NULL);


    //ROACH
    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, "roachConnected", "Is the RoachAcquisitionServer connected to the ROACH's KATCP server", "none",
                                  &getIsStationControllerKATCPConnected, NULL, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "accumulationLength", "number of FFT frames accumulated after the final PFB-FFT stage", "none",
                                  &getAccumulationLength_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "narrowBandChannelSelect", "cannonical coarse FFT bin no. selected for narrow band FFT processing", "none",
                                  &getCoarseChannelSelect_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "frequencyFs", "ADC sample rate", "MHz",
                                 &getFrequencyFs_KATCPCallback, NULL, NULL, 800.0, 800.0, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "sizeOfCoarseFFT", "Size of the the coarse FFT", "no. of input time domain samples",
                                  &getSizeOfCoarseFFT_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "sizeOfFineFFT", "Size of the fine FFTs", "no. of input time domain samples",
                                  &getSizeOfFineFFT_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "coarseFFTShiftMask", "Mask determining the scaling with the FFT stages", "none",
                                  &getCoarseFFTShiftMask_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "attenuationADCChan0", "Attenuation of ADC channel 0", "dB",
                                 &getADCAttenuationChan0_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "attenuationADCChan1", "Attenuation of ADC channel 1", "dB",
                                 &getADCAttenuationChan1_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_katcp(m_pKATCPDispatch, "?startRecording", "start data recording to HDF5", &cKATCPServer::startRecording_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?stopRecording",  "stop data recording to HDF5", &cKATCPServer::stopRecording_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?getRecordingInfo", "get info about current recording.", &cKATCPServer::getRecordingInfo_KATCPCallback);
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
        //Cast to RoachKATPClient point to circumvent ambiguous implicit conversion.
        cRoachKATCPClient::cCallbackInterface* pRoachKATCPClientCallbackHandler
                = dynamic_cast<cRoachKATCPClient::cCallbackInterface*>(&m_oKATCPClientCallbackHandler);
        m_pRoachKATCPClient->registerCallbackHandler(pRoachKATCPClientCallbackHandler);
    }
}

void cKATCPServer::setStationControllerKATCPClient(boost::shared_ptr<cStationControllerKATCPClient> pKATCPClient)
{
    m_pStationControllerKATCPClient = pKATCPClient;

    if(m_pStationControllerKATCPClient.get())
    {
        //Cast to RoachKATPClient point to circumvent ambiguous implicit conversion.
        cStationControllerKATCPClient::cCallbackInterface* pStationControllerKATCPClientCallbackHandler
                = dynamic_cast<cStationControllerKATCPClient::cCallbackInterface*>(&m_oKATCPClientCallbackHandler);
        m_pStationControllerKATCPClient->registerCallbackHandler(pStationControllerKATCPClientCallbackHandler);
    }
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
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%llu", m_pFileWriter->getCurrentFileSize_B());
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

//Get functions for KATCP sensors Station Controller values
int cKATCPServer::getIsStationControllerKATCPConnected(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    if(m_oKATCPClientCallbackHandler.m_bStationControllerKATCPConnected)
        return 1;
    else
        return 0;
}

double cKATCPServer::getRequestedAntennaAz(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dRequestedAntennaAz_deg;
}

double cKATCPServer::getRequestedAntennaEl(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dRequestedAntennaEl_deg;
}

double cKATCPServer::getActualAntennaAz(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaAz_deg;
}

double cKATCPServer::getActualAntennaEl(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaEl_deg;
}

double cKATCPServer::getActualSourceOffsetAz(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dActualSourceOffsetAz_deg;
}

double cKATCPServer::getActualSourceOffsetEl(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dActualSourceOffsetEl_deg;
}

double cKATCPServer::getActualAntennaRA(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaRA_deg;
}

double cKATCPServer::getActualAntennaDec(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaDec_deg;
}

//char* cKATCPServer::getAntennaStatus(struct katcp_dispatch *pD, katcp_acquire *pA)
//{
//    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

//    return m_oKATCPClientCallbackHandler.m_strAntennaStatus.c_str();
//}

double cKATCPServer::getMotorTorqueAzMaster(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueAzMaster_mNm;
}

double cKATCPServer::getMotorTorqueAzSlave(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueAzSlave_mNm;
}

double cKATCPServer::getMotorTorqueElMaster(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueElMaster_mNm;
}

double cKATCPServer::getMotorTorqueElSlave(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueElSlave_mNm;
}

int32_t cKATCPServer::getNoiseDiodeSoftwareState(katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_i32NoiseDiodeSoftwareState;
}

//char* cKATCPServer::getNoiseDiodeSource(struct katcp_dispatch *pD, struct katcp_acquire *pA)
//{
//    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

//    return m_oKATCPClientCallbackHandler.m_strNoiseDiodeSource.c_str();
//}

double cKATCPServer::getNoiseDiodeCurrent(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dNoiseDiodeCurrent_A;
}

double cKATCPServer::getFrequencyRFChan0(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyRFChan0_MHz;
}

double cKATCPServer::getFrequencyRFChan1(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyRFChan1_MHz;
}

double cKATCPServer::getFrequencyLO0Chan0(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan0_MHz;
}

double cKATCPServer::getFrequencyLO0Chan1(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan1_MHz;
}

double cKATCPServer::getFrequencyLO1(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyLO1_MHz;
}

double cKATCPServer::getReceiverBandwidthChan0(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dReceiverBandwidthChan0_MHz;
}

double cKATCPServer::getReceiverBandwidthChan1(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    return m_oKATCPClientCallbackHandler.m_dReceiverBandwidthChan1_MHz;
}

//Get functions for KATCP sensors ROACH values
int cKATCPServer::getIsRoachKATCPConnected(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    if(m_oKATCPClientCallbackHandler.m_bStationControllerKATCPConnected)
        return 1;
    else
        return 0;
}

int32_t cKATCPServer::getAccumulationLength_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_u32AccumulationLength_nFrames;
}

int32_t cKATCPServer::getCoarseChannelSelect_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_u32CoarseChannelSelection;
}

double cKATCPServer::getFrequencyFs_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyFs_MHz;
}

int32_t cKATCPServer::getSizeOfCoarseFFT_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_u32SizeOfCoarseFFT_nSamp;
}

int32_t cKATCPServer::getSizeOfFineFFT_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_u32SizeOfFineFFT_nSamp;
}

int32_t cKATCPServer::getCoarseFFTShiftMask_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_u32CoarseFFTShiftMask;
}

double cKATCPServer::getADCAttenuationChan0_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_dADCAttenuationChan0_dB;
}

double cKATCPServer::getADCAttenuationChan1_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_dADCAttenuationChan1_dB;
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
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_bRoachKATCPConnected = bConnected;
}

void cKATCPServer::cKATCPClientCallbackHandler::startRecording_callback(const string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void cKATCPServer::cKATCPClientCallbackHandler::stopRecording_callback()
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void  cKATCPServer::cKATCPClientCallbackHandler::requestedAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dRequestedAntennaAz_deg = dAzimuth_deg;
}

void  cKATCPServer::cKATCPClientCallbackHandler::requestedAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dRequestedAntennaEl_deg = dElevation_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dActualAntennaAz_deg = dAzimuth_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dActualAntennaEl_deg = dElevation_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualSourceOffsetAz_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dActualSourceOffsetAz_deg = dAzimuthOffset_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualSourceOffsetEl_callback(int64_t i64Timestamp_us, double dElevationOffset_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dActualSourceOffsetEl_deg = dElevationOffset_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaRA_callback(int64_t i64Timestamp_us, double dRighAscension_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dActualAntennaRA_deg = dRighAscension_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaDec_callback(int64_t i64Timestamp_us, double dDeclination_deg)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dActualAntennaDec_deg = dDeclination_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaStatus_callback(int64_t i64Timestamp_us, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_strAntennaStatus = strStatus;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueAzMaster_callback(int64_t i64Timestamp_us, double dAzMaster_mNm)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dMotorTorqueAzMaster_mNm = dAzMaster_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueAzSlave_callback(int64_t i64Timestamp_us, double dAzSlave_mNm)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dMotorTorqueAzSlave_mNm = dAzSlave_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueElMaster_callback(int64_t i64Timestamp_us, double dElMaster_mNm)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dMotorTorqueElMaster_mNm = dElMaster_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueElSlave_callback(int64_t i64Timestamp_us, double dElSlave_mNm)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dMotorTorqueAzSlave_mNm = dElSlave_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::appliedPointingModel_callback(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    //Todo
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_i32NoiseDiodeSoftwareState = i32NoiseDiodeState;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeSource_callback(int64_t i64Timestamp_us, const string &strNoiseDiodeSource)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_strNoiseDiodeSource = strNoiseDiodeSource;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dNoiseDiodeCurrent_A = dNoiseDiodeCurrent_A;
}

void cKATCPServer::cKATCPClientCallbackHandler::sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg)
{
    //Todo
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyRFChan0_callback(int64_t i64Timestamp_us, double dFrequencyRFChan0_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dFrequencyRFChan0_MHz = dFrequencyRFChan0_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyRFChan1_callback(int64_t i64Timestamp_us, double dFrequencyRFChan1_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dFrequencyRFChan1_MHz = dFrequencyRFChan1_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLO0Chan0_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dFrequencyLO0Chan0_MHz = dFrequencyLO0Chan0_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLO0Chan1_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dFrequencyLO0Chan1_MHz = dFrequencyLO0Chan1_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLO1_callback(int64_t i64Timestamp_us, double dFrequencyLO1_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dFrequencyLO1_MHz = dFrequencyLO1_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::receiverBandwidthChan0_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dReceiverBandwidthChan0_MHz = dReceiverBandwidthChan0_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::receiverBandwidthChan1_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    m_dReceiverBandwidthChan1_MHz = dReceiverBandwidthChan1_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_u32AccumulationLength_nFrames = u32NFrames;
}

void cKATCPServer::cKATCPClientCallbackHandler::coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_u32CoarseChannelSelection = u32ChannelNo;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyFs_callback(double dFrequencyFs_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_dFrequencyFs_MHz = dFrequencyFs_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_u32SizeOfCoarseFFT_nSamp = u32SizeOfCoarseFFT_nSamp;
}

void cKATCPServer::cKATCPClientCallbackHandler::sizeOfFineFFT_callback(uint32_t u32SizeOfFineFFT_nSamp)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_u32SizeOfFineFFT_nSamp = u32SizeOfFineFFT_nSamp;
}

void cKATCPServer::cKATCPClientCallbackHandler::coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_u32CoarseFFTShiftMask = u32ShiftMask;
}

void cKATCPServer::cKATCPClientCallbackHandler::attenuationADCChan0_callback(int64_t i64Timestamp_us, double dAttenuationChan0_dB)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_dADCAttenuationChan0_dB = dAttenuationChan0_dB;
}

void cKATCPServer::cKATCPClientCallbackHandler::attenuationADCChan1_callback(int64_t i64Timestamp_us, double dAttenuationChan1_dB)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_dADCAttenuationChan1_dB = dAttenuationChan1_dB;
}
