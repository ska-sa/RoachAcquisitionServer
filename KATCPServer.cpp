//System includes
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <cstdlib>

//Library includes
extern "C" {
#include <katpriv.h>
}

#include <boost/filesystem.hpp>

//Local includes
#include "KATCPServer.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"
#include "AVNUtilLibs/Sockets/InterruptibleBlockingSockets/InterruptibleBlockingTCPSocket.h"
#include "AVNUtilLibs/DirectoryContents/DirectoryContents.h"
#include "AVNUtilLibs/Filename/Filename.h"

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
std::string                                         cKATCPServer::m_strRoachGatewareDirectory;

cKATCPServer::cKATCPServer(const string &strInterface, uint16_t u16Port, uint32_t u32MaxClients, const string &strRoachGatewareDirectory)
{
    m_strRoachGatewareDirectory = strRoachGatewareDirectory;
    startServer(strInterface, u16Port, u32MaxClients);
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

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "requestedAntennaAz", "requested antenna azimuth after the pointing model", "degrees",
                                 &getRequestedAntennaAz, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "requestedAntennaEl", "requested antenna elevation after the pointing model", "degrees",
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

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "frequencyRFChan1", "the centre frequency of RF mapped to final IF output channel 1", "MHz",
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
                                  &getIsRoachKATCPConnected, NULL, NULL);

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, "roachStokesEnabled", "Is the ROACH spitting out LRQU data. (Otherwise LRPP)", "none",
                                  &getStokesEnabled, NULL, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachAccumulationLength", "number of FFT frames accumulated after the final PFB-FFT stage", "none",
                                  &getAccumulationLength_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachCoarseChannelSelect", "cannonical coarse FFT bin no. selected for narrow band FFT processing", "none",
                                  &getCoarseChannelSelect_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "roachFrequencyFs", "ADC sample rate", "MHz",
                                 &getFrequencyFs_KATCPCallback, NULL, NULL, 800.0, 800.0, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachSizeOfCoarseFFT", "Size of the the coarse FFT", "no. of input time domain samples",
                                  &getSizeOfCoarseFFT_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachSizeOfFineFFT", "Size of the fine FFTs", "no. of input time domain samples",
                                  &getSizeOfFineFFT_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachCoarseFFTShiftMask", "Mask determining the scaling with the FFT stages", "none",
                                  &getCoarseFFTShiftMask_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "roachAttenuationADCChan0", "Attenuation of ADC channel 0", "dB",
                                 &getADCAttenuationChan0_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_double_sensor_katcp(m_pKATCPDispatch, 0, "roachAttenuationADCChan1", "Attenuation of ADC channel 1", "dB",
                                 &getADCAttenuationChan1_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, "roachNoiseDiodeEnabled", "Is the Roach's noise diode control enabled", "none",
                                  &getNoiseDiodeEnabled_KATCPCallback, NULL, NULL);

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, "roachNoiseDiodeDutyCycleEnabled", "Is the Roach generating a sample-clock synchronous duty cycle for the noise diode", "none",
                                  &getNoiseDiodeDutyCycleEnabled_KATCPCallback, NULL, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachNoiseDiodeDutyCycleOnDuration", "Duration of the ON part of the Roach noise diode duty cycle", "no. of accumulations",
                                  &getNoiseDiodeDutyCycleOnDuration_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachNoiseDiodeDutyCycleOffDuration", "Duration of the OFF part of the Roach noise diode duty cycle", "no. of accumulations",
                                  &getNoiseDiodeDutyCycleOffDuration_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachOverflowRegs", "Overflow registers", "none",
                                  &getOverflowsRegs_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, "roachEth10GbEUp", "Is the relevant Roach 10GbE port for the current gateware link up", "none",
                                  &getEth10GbEUp_KATCPCallback, NULL, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachPPSCount", "A count of the received PPS edges", "none",
                                  &getPPSCount_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0, "roachClockFrequency", "The clock frequency of the FPGA", "Hz",
                                  &getClockFrequency_KATCPCallback, NULL, NULL, 200000000, 200000000, NULL);

    register_katcp(m_pKATCPDispatch, "?startRecording", "start data recording to HDF5", &cKATCPServer::startRecording_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?stopRecording",  "stop data recording to HDF5", &cKATCPServer::stopRecording_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?getRecordingInfo", "get info about current recording.", &cKATCPServer::getRecordingInfo_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?getRecordingStatus", "is recording or not", &cKATCPServer::getRecordingStatus_KATCPCallback);

    register_katcp(m_pKATCPDispatch, "?getRoachGatewareList", "Retrieve a list of gateware that can be used with the Roach", &cKATCPServer::getRoachGatewareList_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?programRoach", "Program the Roach by specifying a launch script", &cKATCPServer::roachProgram_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachStokesEnabled", "Enable stokes calculation to output LRQU (otherwise LRPP)", &cKATCPServer::roachSetStokesEnabled_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachAccumulationLength", "number of accumulations after final FFT stage", &cKATCPServer::roachSetAccumulationLength_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachCoarseChannelSelect", "coarse FFT cannonical channel number to use for fine FFT input", &cKATCPServer::roachSetCoarseChannelSelect_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachCoarseFFTMask", "Shift mask for coarse FFT", &cKATCPServer::roachSetCoarseFFTShiftMask_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachADC0Attenuation", "Attenuation of ADC0 (0.5 dB per step max 31.5 dB)", &cKATCPServer::roachSetADC0Attenuation_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachADC1Attenuation", "Attenuation of ADC1 (0.5 dB per step max 31.5 dB)", &cKATCPServer::roachSetADC1Attenuation_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachNoiseDiodeEnabled", "Enable noise diode", &cKATCPServer::roachSetNoiseDiodeEnabled_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachNoiseDiodeDutyCycleEnabled", "Enable noise diode duty cycle mode", &cKATCPServer::roachSetNoiseDiodeDutyCycleEnabled_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachNoiseDiodeDutyCycleOnDuration", "number of accumulations that noise diode is on in duty cycle mode", &cKATCPServer::roachSetNoiseDiodeDutyCycleOnDuration_KATCPCallback);
    register_katcp(m_pKATCPDispatch, "?setRoachNoiseDiodeDutyCycleOffDuration", "number of accumulations that noise diode is off in duty cycle mode", &cKATCPServer::roachSetNoiseDiodeDutyCycleOffDuration_KATCPCallback);

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

//Controlling of Roach
int32_t cKATCPServer::getRoachGatewareList_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    //Get all python files in the Launcher directory
    vector<string> vstrFilenames;
    vector<string> vstrValidFilenames;
    try
    {
        cDirectoryContents oFiles(m_strRoachGatewareDirectory, string(".py"), true);
        vstrFilenames = oFiles.getContents();
    }
    catch(cDirectoryContentsException oE)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "getRoachGatewareList: Could not access directory.");
        return KATCP_RESULT_FAIL;
    }

    //Each launcher script should have a first line "#!RoachSpectrometerLauncher" to be considered valid.
    for(uint32_t ui = 0; ui < vstrFilenames.size(); ui++)
    {
        ifstream oLauncherFile;
        oLauncherFile.open(vstrFilenames[ui].c_str());

        //Cut full path down to filename only
        vstrFilenames[ui] = AVN::getFilenameOnly(vstrFilenames[ui]);

        if(!oLauncherFile.is_open())
        {
            cout << "cKATCPServer::getRoachGatewareList_KATCPCallback(): Unable to open Roach gateware launcher file \"" << vstrFilenames[ui] << "\". Ignoring." << endl;
            continue;
        }

        string str1stLine;
        getline(oLauncherFile, str1stLine);

        //Check that the first line has the required description
        if(str1stLine.compare("#!RoachSpectrometerLauncher"))
        {
            cout << "cKATCPServer::getRoachGatewareList_KATCPCallback(): File \"" << vstrFilenames[ui] << "\". is does not appear to be a valid Roach gateware launcher file. Ignoring." << endl;
            continue;
        }

        vstrValidFilenames.push_back(vstrFilenames[ui]);
    }

    if(vstrValidFilenames.size())
    {
        send_katcp( pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#roachGatewareList");
        for(int32_t i = 0; i < (int32_t)vstrValidFilenames.size() - 1; i++)
        {
            append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, "%s", vstrValidFilenames[i].c_str());
        }
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, "%s", vstrValidFilenames[vstrValidFilenames.size() - 1].c_str());
    }
    else
    {
        send_katcp( pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, "#roachGatewareList");
    }

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::roachProgram_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "roachProgram: Incorrect number of arguments.");
        return KATCP_RESULT_FAIL;
    }

    stringstream oSS;
    oSS << "cd ";
    oSS << m_strRoachGatewareDirectory;
    oSS << " && python ";
    oSS << arg_string_katcp(pKATCPDispatch, 1);

    cout << "cKATCPServer::roachProgram_KATCPCallback(): Running system command \"" << oSS.str() << "\" to program Roach" << endl;

    system(oSS.str().c_str());

    cout << "cKATCPServer::roachProgram_KATCPCallback(): Done." << endl;

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::roachSetStokesEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachStokesEnabled: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("stokes_enable"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetAccumulationLength_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachAccumulationLength: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("accumulation_length"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetCoarseChannelSelect_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachCoarseChannelSelect: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("coarse_channel_select"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetCoarseFFTShiftMask_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachCoarseFFTMask: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("coarse_fft_shift_mask"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetADC0Attenuation_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachADC0Attenuation: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("adc0_atten"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetADC1Attenuation_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachADC1Attenuation: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("adc1_atten"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetNoiseDiodeEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachNoiseDiodeEnabled: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("noise_diode_en"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetNoiseDiodeDutyCycleEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachNoiseDiodeDutyCycleEnabled: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("noise_diode_duty_cycle_en"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetNoiseDiodeDutyCycleOnDuration_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachNoiseDiodeDutyCycleOnDuration: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("noise_diode_on_length"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
}

int32_t cKATCPServer::roachSetNoiseDiodeDutyCycleOffDuration_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "setRoachNoiseDiodeDutyCycleOffDuration: No KATCPclient object for Roach control set.");
        return KATCP_RESULT_FAIL;
    }

    if(m_pRoachKATCPClient->writeRoachRegister(string("noise_diode_off_length"), strtoul(arg_string_katcp(pKATCPDispatch, 1), NULL, 10)))
    {
        return KATCP_RESULT_OK;
    }
    else
    {
        return KATCP_RESULT_FAIL;
    }
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
int32_t cKATCPServer::getIsRoachKATCPConnected(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    if(m_oKATCPClientCallbackHandler.m_bRoachKATCPConnected)
        return 1;
    else
        return 0;
}

int32_t cKATCPServer::getStokesEnabled(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    if(m_oKATCPClientCallbackHandler.m_bStokesEnabled)
        return 1;
    else
        return 0;
}

int32_t cKATCPServer::getAccumulationLength_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32AccumulationLength_nFrames;
}

int32_t cKATCPServer::getCoarseChannelSelect_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32CoarseChannelSelection;
}

double cKATCPServer::getFrequencyFs_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyFs_MHz;
}

int32_t cKATCPServer::getSizeOfCoarseFFT_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32SizeOfCoarseFFT_nSamp;
}

int32_t cKATCPServer::getSizeOfFineFFT_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32SizeOfFineFFT_nSamp;
}

int32_t cKATCPServer::getCoarseFFTShiftMask_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32CoarseFFTShiftMask;
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

int32_t cKATCPServer::getNoiseDiodeEnabled_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_bNoiseDiodeEnabled;
}

int32_t cKATCPServer::getNoiseDiodeDutyCycleEnabled_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_bNoiseDiodeDutyCycleEnabled;
}

int32_t cKATCPServer::getNoiseDiodeDutyCycleOnDuration_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32NoiseDiodeDutyCycleOnDuration_nAccums;
}

int32_t cKATCPServer::getNoiseDiodeDutyCycleOffDuration_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32NoiseDiodeDutyCycleOffDuration_nAccums;
}

int32_t  cKATCPServer::getOverflowsRegs_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32OverflowRegs;
}

int32_t cKATCPServer::getEth10GbEUp_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_bEth10GbEUp;
}

int32_t cKATCPServer::getPPSCount_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32PPSCount;
}

int32_t cKATCPServer::getClockFrequency_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32ClockFrequency;
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

void cKATCPServer::cKATCPClientCallbackHandler::connected_callback(bool bConnected, const std::string &strHostAddress, uint16_t u16Port, const std::string &strDescription)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

    cout << "cKATCPServer::cKATCPClientCallbackHandler::connected_callback(): " << strDescription << endl;


    if(!strDescription.compare("roach"))
    {
        m_bRoachKATCPConnected = bConnected;

        if(m_bRoachKATCPConnected)
        {
            stringstream oSS;
            oSS << strHostAddress;
            oSS << ":";
            oSS << u16Port;

            m_strRoachAddress = oSS.str();
        }
    }

    if(!strDescription.compare("stationController"))
    {
        m_bStationControllerKATCPConnected = bConnected;

        if(m_bStationControllerKATCPConnected)
        {
            stringstream oSS;
            oSS << strHostAddress;
            oSS << ":";
            oSS << u16Port;

            m_strStationControllerAddress = oSS.str();
        }
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

void cKATCPServer::cKATCPClientCallbackHandler::stokesEnabled_callback(int64_t i64Timestamp_us, bool bEnabled)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_bStokesEnabled = bEnabled;
}

void cKATCPServer::cKATCPClientCallbackHandler::accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32AccumulationLength_nFrames = u32NFrames;
}

void cKATCPServer::cKATCPClientCallbackHandler::coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32CoarseChannelSelection = u32ChannelNo;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyFs_callback(double dFrequencyFs_MHz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_dFrequencyFs_MHz = dFrequencyFs_MHz;
}

void cKATCPServer::cKATCPClientCallbackHandler::sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32SizeOfCoarseFFT_nSamp = u32SizeOfCoarseFFT_nSamp;
}

void cKATCPServer::cKATCPClientCallbackHandler::sizeOfFineFFT_callback(uint32_t u32SizeOfFineFFT_nSamp)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32SizeOfFineFFT_nSamp = u32SizeOfFineFFT_nSamp;
}

void cKATCPServer::cKATCPClientCallbackHandler::coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32CoarseFFTShiftMask = u32ShiftMask;
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

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoideDiodeEnabled)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_bNoiseDiodeEnabled = bNoideDiodeEnabled;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeDutyCycleEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeDutyCyleEnabled)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_bNoiseDiodeDutyCycleEnabled = bNoiseDiodeDutyCyleEnabled;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeDutyCycleOnDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32NoiseDiodeDutyCycleOnDuration_nAccums = u32NAccums;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeDutyCycleOffDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32NoiseDiodeDutyCycleOffDuration_nAccums = u32NAccums;
}

void cKATCPServer::cKATCPClientCallbackHandler::overflowsRegs_callback(int64_t i64Timestamp_us, uint32_t u32OverflowRegs)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32OverflowRegs = u32OverflowRegs;
}

void cKATCPServer::cKATCPClientCallbackHandler::eth10GbEUp_callback(int64_t i64Timestamp_us, bool bEth10GbEUp)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_bEth10GbEUp = bEth10GbEUp;
}

void cKATCPServer::cKATCPClientCallbackHandler::ppsCount_callback(int64_t i64Timestamp_us, uint32_t u32PPSCount)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32PPSCount = u32PPSCount;
}

void cKATCPServer::cKATCPClientCallbackHandler::clockFrequency_callback(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_i32ClockFrequency = u32ClockFrequency_Hz;
}