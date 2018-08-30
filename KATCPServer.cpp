//System includes
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

//Library includes
extern "C" {
#include <katpriv.h>
}

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

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
boost::shared_ptr<boost::thread>                    cKATCPServer::m_pInitialSensorDataThread;
std::string                                         cKATCPServer::m_strListenInterface;
uint16_t                                            cKATCPServer::m_u16Port;
uint32_t                                            cKATCPServer::m_u32MaxClients;
std::string                                         cKATCPServer::m_strRoachGatewareDirectory;
struct katcp_acquire                                *cKATCPServer::m_pKAStationControllerConnected;
struct katcp_acquire                                *cKATCPServer::m_pKARoachConnected;
struct katcp_acquire                                *cKATCPServer::m_pKA10GbEUP;

//Extract function for boolean sensors which don't care whether or not they're true or false.
//Courtesy Marc Welz
int extract_dontcare_boolean_katcp(struct katcp_dispatch *d, struct katcp_sensor *sn)
{
  set_status_sensor_katcp(sn, KATCP_STATUS_NOMINAL);
  return 0;
}




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
    add_version_katcp(m_pKATCPDispatch, const_cast<char*>("RoachAcquisitionServer"), 0, const_cast<char*>("0.9"), &oSSCompileTime.str()[0]);

    //Declare sensors
    //Station controller

    m_pKAStationControllerConnected = setup_boolean_acquire_katcp(m_pKATCPDispatch, &getIsStationControllerKATCPConnected, NULL, NULL);
    register_direct_multi_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("stationControllerConnected"),
                                  const_cast<char*>("Is the RoachAcquisitionServer connected to the StationController's KATCP server"),
                                  const_cast<char*>("none"),
                                  m_pKAStationControllerConnected);

    //Recording info.
    // TODO: figure out sensible min and max values for these.
    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingStartTime"),
                                  const_cast<char*>("Unix time at which the current recording started."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingStartTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingElapsedTime"),
                                  const_cast<char*>("Duration of current recording."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingElapsedTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingStopTime"),
                                  const_cast<char*>("Unix time at which the current recording is scheduled to stop."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingStopTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingRemainingTime"),
                                  const_cast<char*>("Time until the current recording is scheduled to stop."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingRemainingTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingFileSize"),
                                  const_cast<char*>("Current size of file being recorded."),
                                  const_cast<char*>("bytes"),
                                  &getRecordingFileSize_KATCPCallback, NULL, NULL, 0, 10e9, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingDiskSpace"),
                                  const_cast<char*>("Amount of disk space still available on SDB."),
                                  const_cast<char*>("bytes"),
                                  &getDiskSpace_KATCPCallback, NULL, NULL, 100e9, 20e12, NULL);

    //ROACH

    m_pKARoachConnected = setup_boolean_acquire_katcp(m_pKATCPDispatch, &getIsRoachKATCPConnected, NULL, NULL);
    register_direct_multi_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachConnected"),
                                  const_cast<char*>("Is the RoachAcquisitionServer connected to the ROACH's KATCP server"),
                                  const_cast<char*>("none"),
                                  m_pKARoachConnected);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachAccumulationLength"),
                                  const_cast<char*>("number of FFT frames accumulated after the final PFB-FFT stage"),
                                  const_cast<char*>("none"),
                                  &getAccumulationLength_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachCoarseChannelSelect"),
                                  const_cast<char*>("canonical coarse FFT bin no. selected for narrow band FFT processing"),
                                  const_cast<char*>("none"),
                                  &getCoarseChannelSelect_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("roachFrequencyFs"),
                                 const_cast<char*>("ADC sample rate"),
                                 const_cast<char*>("Hz"),
                                 &getFrequencyFs_KATCPCallback, NULL, NULL, 799999999, 800000001, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachSizeOfCoarseFFT"),
                                  const_cast<char*>("Size of the the coarse FFT"),
                                  const_cast<char*>("no. of input time domain samples"),
                                  &getSizeOfCoarseFFT_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachSizeOfFineFFT"),
                                  const_cast<char*>("Size of the fine FFTs"),
                                  const_cast<char*>("no. of input time domain samples"),
                                  &getSizeOfFineFFT_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachNumFrequencyChannels"),
                                  const_cast<char*>("Number of frequency channels"),
                                  const_cast<char*>("freq channels"),
                                  &getNumberChannels_KATCPCallback, NULL, NULL, 1023, 4097, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachCoarseFFTShiftMask"),
                                  const_cast<char*>("Mask determining the scaling with the FFT stages"),
                                  const_cast<char*>("none"),
                                  &getCoarseFFTShiftMask_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("roachDSPGain"),
                                 const_cast<char*>("Digital gain in the ROACH signal chain"),
                                 const_cast<char*>("none"),
                                 &getDspGain_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("roachAttenuationADCChan0"),
                                 const_cast<char*>("Attenuation of ADC channel 0"),
                                 const_cast<char*>("dB"),
                                 &getADCAttenuationChan0_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("roachAttenuationADCChan1"),
                                 const_cast<char*>("Attenuation of ADC channel 1"),
                                 const_cast<char*>("dB"),
                                 &getADCAttenuationChan1_KATCPCallback, NULL, NULL, 0, 31.5, NULL); //Assume KATADC

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachNoiseDiodeEnabled"),
                                  const_cast<char*>("Is the Roach's noise diode control enabled"),
                                  const_cast<char*>("none"),
                                  &getNoiseDiodeEnabled_KATCPCallback, NULL, NULL, 0, 1, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachNoiseDiodeDutyCycleEnabled"),
                                  const_cast<char*>("Is the Roach's noise diode duty cycle mode enabled"),
                                  const_cast<char*>("none"),
                                  &getNoiseDiodeDutyCycleEnabled_KATCPCallback, NULL, NULL, 0, 1, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachNoiseDiodeDutyCycleOnDuration"),
                                  const_cast<char*>("Duration of the ON part of the Roach noise diode duty cycle"),
                                  const_cast<char*>("no. of accumulations"),
                                  &getNoiseDiodeDutyCycleOnDuration_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachNoiseDiodeDutyCycleOffDuration"),
                                  const_cast<char*>("Duration of the OFF part of the Roach noise diode duty cycle"),
                                  const_cast<char*>("no. of accumulations"),
                                  &getNoiseDiodeDutyCycleOffDuration_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachOverflowRegs"),
                                  const_cast<char*>("Overflow registers"),
                                  const_cast<char*>("none"),
                                  &getOverflowsRegs_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    m_pKA10GbEUP = setup_boolean_acquire_katcp(m_pKATCPDispatch, &getEth10GbEUp_KATCPCallback, NULL, NULL);
    register_direct_multi_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachEth10GbEUp"),
                                  const_cast<char*>("Is the relevant Roach 10GbE port for the current gateware link up"),
                                  const_cast<char*>("none"),
                                  m_pKA10GbEUP);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachPPSCount"),
                                  const_cast<char*>("A count of the received PPS edges"),
                                  const_cast<char*>("none"),
                                  &getPPSCount_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachClockFrequency"),
                                  const_cast<char*>("The clock frequency of the FPGA"),
                                  const_cast<char*>("Hz"),
                                  &getClockFrequency_KATCPCallback, NULL, NULL, 199999999, 200000001, NULL);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?startRecording"),
                   const_cast<char*>("start data recording to HDF5"),
                   &cKATCPServer::startRecording_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?stopRecording"),
                   const_cast<char*>("stop data recording to HDF5"),
                   &cKATCPServer::stopRecording_KATCPCallback);

    // For backwards compatibility
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?getRecordingInfo"),
                   const_cast<char*>("get info about current recording."),
                   &cKATCPServer::getRecordingInfo_KATCPCallback);
//end

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?getCurrentFilename"),
                   const_cast<char*>("Get current filename."),
                   &cKATCPServer::getCurrentFilename_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?getRecordingStatus"),
                   const_cast<char*>("is recording or not"),
                   &cKATCPServer::getRecordingStatus_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?getRoachGatewareList"),
                   const_cast<char*>("Retrieve a list of gateware that can be used with the Roach"),
                   &cKATCPServer::getRoachGatewareList_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?programRoach"),
                   const_cast<char*>("Program the Roach by specifying a launch script"),
                   &cKATCPServer::roachProgram_KATCPCallback);
/*
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachStokesEnabled"),
                   const_cast<char*>("Enable stokes calculation to output LRQU (otherwise LRPP)"),
                   &cKATCPServer::roachSetStokesEnabled_KATCPCallback);
*/
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachAccumulationLength"),
                   const_cast<char*>("number of accumulations after final FFT stage"),
                   &cKATCPServer::roachSetAccumulationLength_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachCoarseChannelSelect"),
                   const_cast<char*>("coarse FFT canonical channel number to use for fine FFT input"),
                   &cKATCPServer::roachSetCoarseChannelSelect_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachCoarseFFTMask"),
                   const_cast<char*>("Shift mask for coarse FFT"),
                   &cKATCPServer::roachSetCoarseFFTShiftMask_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachDspGain"),
                   const_cast<char*>("Digital gain for DSP signal chain"),
                   &cKATCPServer::roachSetDspGain_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachADC0Attenuation"),
                   const_cast<char*>("Attenuation of ADC0 (0.5 dB per step max 31.5 dB)"),
                   &cKATCPServer::roachSetADC0Attenuation_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachADC1Attenuation"),
                   const_cast<char*>("Attenuation of ADC1 (0.5 dB per step max 31.5 dB)"),
                   &cKATCPServer::roachSetADC1Attenuation_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachNoiseDiodeEnabled"),
                   const_cast<char*>("Enable noise diode"),
                   &cKATCPServer::roachSetNoiseDiodeEnabled_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachNoiseDiodeDutyCycleEnabled"),
                   const_cast<char*>("Enable noise diode duty cycle mode"),
                   &cKATCPServer::roachSetNoiseDiodeDutyCycleEnabled_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachNoiseDiodeDutyCycleOnDuration"),
                   const_cast<char*>("number of accumulations that noise diode is on in duty cycle mode"),
                   &cKATCPServer::roachSetNoiseDiodeDutyCycleOnDuration_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachNoiseDiodeDutyCycleOffDuration"),
                   const_cast<char*>("number of accumulations that noise diode is off in duty cycle mode"),
                   &cKATCPServer::roachSetNoiseDiodeDutyCycleOffDuration_KATCPCallback);

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


int32_t cKATCPServer::startRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
/*
 * TODO: Need to make this command more clear.
 */
{
    cout << "cKATCPServer::startRecording_KATCPCallback()" << endl;

    if(!m_pFileWriter.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("startRecording: No file recorder object set."));
        return KATCP_RESULT_FAIL;
    }

    if(i32ArgC > 4)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, const_cast<char*>("startRecording: Warning %i redundant argument(s) for stop recording, ignoring."), i32ArgC - 4);
    }
    string strFileSuffix("");
    int64_t i64StartTime_us = 0;
    int64_t i64Duration_us = 0;

    if(i32ArgC >= 2)
    {
        try
        {
            strFileSuffix = string(arg_string_katcp(pKATCPDispatch, 1));
        }
        catch(std::logic_error &oError)
        {
            cout << "cKATCPServer::startRecording_callback() Error unable to intepret filename suffix. Ignoring" << endl;
            cout << "Error was " << oError.what() << endl;
        }
    }

    if(i32ArgC >= 3)
        i64StartTime_us = strtoll(arg_string_katcp(pKATCPDispatch, 2), NULL, 10);

    if(i32ArgC >= 4)
        i64Duration_us = strtoll(arg_string_katcp(pKATCPDispatch, 3), NULL, 10);

    // Little bit of data validation never hurt anyone.
    if ( i64StartTime_us < time(0)*1e6 )
    {
        cout << "cKATCPServer::startRecording_callback() Warning: Time specified is in the past. Starting recording immediately." << endl;
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, const_cast<char*>("startRecording: Warning: %i is in the past, starting recording immediately."), i64StartTime_us);
        i64StartTime_us = 0;
    }


    cout << "--------------------------------------------------------------" << endl;
    cout << "cKATCPServer::startRecording_callback() Got request to record:" << endl;
    cout << "File suffix = " << strFileSuffix << endl;
    cout << "Start time  = " << i64StartTime_us << " (" << AVN::stringFromTimestamp_full(i64StartTime_us) << ")" << endl;
    cout << "Duration    = " << i64Duration_us << " (" << AVN::stringFromTimeDuration(i64Duration_us) << ")" << endl;
    cout << "--------------------------------------------------------------" << endl;

    m_pFileWriter->startRecording(strFileSuffix, i64StartTime_us, i64Duration_us);
    if (m_pStationControllerKATCPClient) //i.e. if the thing has actually been connected. Otherwise it segfaults.
    {
        m_pStationControllerKATCPClient->subscribeSensorData();
    }



    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::stopRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    cout << "cKATCPServer::stopRecording_KATCPCallback()" << endl;

    if(!m_pFileWriter.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("stopRecording: No file recorder object set."));
        return KATCP_RESULT_FAIL;
    }

    if(i32ArgC > 1)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, const_cast<char*>("stopRecording: Warning, %i redundant argument(s) for stop recording, ignoring."), i32ArgC - 1);
    }

    if(!m_pFileWriter->isRecordingEnabled())
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, const_cast<char*>("stopRecording: Warning, server is not currently recording"));
        return KATCP_RESULT_FAIL;
    }

    m_pFileWriter->stopRecording();
    if (m_pStationControllerKATCPClient) //i.e. if the thing has actually been connected. Otherwise it segfaults.
    {
        // I don't actually think that this should be here.
        //m_pStationControllerKATCPClient->unsubscribeSensorData();
    }

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::getRecordingStatus_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("#recordingStarted"));
    }
    else
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("#recordingStopped"));
    }

    return KATCP_RESULT_OK;
}


int32_t cKATCPServer::getCurrentFilename_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        send_katcp( pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#getCurrentFilename");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("%s"), m_pFileWriter->getFilename().c_str());
    }
    else
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#getCurrentFilename");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("not recording"));
    }

    return KATCP_RESULT_OK;
}


int32_t cKATCPServer::getRecordingStartTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return int(m_pFileWriter->getRecordingStartTime_us()/1e6);
    }
    else
    {
        return 0;
    }
}


int32_t cKATCPServer::getRecordingElapsedTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return int(m_pFileWriter->getRecordedDuration_us()/1e6);
    }
    else
    {
        return 0;
    }
}


int32_t cKATCPServer::getRecordingStopTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return int(m_pFileWriter->getRecordingStopTime_us()/1e6);
    }
    else
    {
        return 0;
    }
}


int32_t cKATCPServer::getRecordingRemainingTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return int(m_pFileWriter->getRecordingTimeLeft_us()/1e6);
    }
    else
    {
        return 0;
    }
}


double cKATCPServer::getRecordingFileSize_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return (double)(m_pFileWriter->getCurrentFileSize_B());
    }
    else
    {
        return 0.0;
    }
}

double cKATCPServer::getDiskSpace_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    return boost::filesystem::space(m_pFileWriter->getRecordingDirectory()).available;
}


int32_t cKATCPServer::getRecordingInfo_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        send_katcp( pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#recordingInfo");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%s"), m_pFileWriter->getFilename().c_str());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%lli"), m_pFileWriter->getRecordingStartTime_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%lli"), m_pFileWriter->getRecordedDuration_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%lli"), m_pFileWriter->getRecordingStopTime_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%lli"), m_pFileWriter->getRecordingTimeLeft_us());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%llu"), m_pFileWriter->getCurrentFileSize_B());
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("%llu"), boost::filesystem::space(m_pFileWriter->getRecordingDirectory()).available);
    }
    else
    {
        send_katcp(pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#recordingInfo");
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("not recording"));
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("%llu"), boost::filesystem::space(m_pFileWriter->getRecordingDirectory()).available);
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
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("getRoachGatewareList: Could not access directory."));
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
        for(int32_t i = 0; i < (int32_t)vstrValidFilenames.size(); i++)
        {
            //For some reason without going through this for loop, something funny slips into the list. Worth investigating.
            cout << "cKATCPServer::getRoachGatewareList_KATCPCallback(): Found Gateware [" << vstrValidFilenames[i].c_str() << "]" << endl;
        }

        send_katcp( pKATCPDispatch, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#getRoachGatewareList");

        for(int32_t i = 0; i < (int32_t)vstrValidFilenames.size() - 1; i++)
        {
            append_args_katcp(pKATCPDispatch, KATCP_FLAG_STRING, const_cast<char*>("%s"), vstrValidFilenames[i].c_str());
        }
        append_args_katcp(pKATCPDispatch, KATCP_FLAG_LAST | KATCP_FLAG_STRING, const_cast<char*>("%s"), vstrValidFilenames[vstrValidFilenames.size() - 1].c_str());
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
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("roachProgram: Incorrect number of arguments."));
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

/*
int32_t cKATCPServer::roachSetStokesEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachStokesEnabled: No KATCPclient object for Roach control set."));
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
*/

int32_t cKATCPServer::roachSetAccumulationLength_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachAccumulationLength: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachAccumulationLength: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachCoarseChannelSelect: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachCoarseChannelSelect: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachCoarseFFTMask: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachCoarseFFTMask: No KATCPclient object for Roach control set."));
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


int32_t cKATCPServer::roachSetDspGain_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachDspGain: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachDspGain: No KATCPclient object for Roach control set."));
        return KATCP_RESULT_FAIL;
    }

    // digital_gain register has a binary-point of 12 bits
    double dRequestedGain = strtod(arg_string_katcp(pKATCPDispatch, 1), NULL) / pow(2,12);

    if (dRequestedGain < 0)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachDspGain: Cannot set a negative gain."));
        return KATCP_RESULT_FAIL;
    }

    // digital_gain register is unsigned. Check for positive numbers.
    uint32_t ui32Gain = int(dRequestedGain * (2^12));

    if(m_pRoachKATCPClient->writeRoachRegister(string("digital_gain"), ui32Gain))
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachADC0Attenuation: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachADC0Attenuation: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachADC1Attenuation: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachADC1Attenuation: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeEnabled: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeEnabled: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeDutyCycleEnabled: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeDutyCycleEnabled: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeDutyCycleOnDuration: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeDutyCycleOnDuration: No KATCPclient object for Roach control set."));
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
    if(i32ArgC != 2)
    {
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeDutyCycleOffDuration: Incorrect number of arguments."));
        return KATCP_RESULT_FAIL;
    }

    if(!m_pRoachKATCPClient.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("setRoachNoiseDiodeDutyCycleOffDuration: No KATCPclient object for Roach control set."));
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

//Get functions for KATCP sensors ROACH values
int32_t cKATCPServer::getIsRoachKATCPConnected(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    if(m_oKATCPClientCallbackHandler.m_bRoachKATCPConnected)
        return 1;
    else
        return 0;
}

/*
int32_t cKATCPServer::getStokesEnabled(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    if(m_oKATCPClientCallbackHandler.m_bStokesEnabled)
        return 1;
    else
        return 0;
}
*/

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

    return m_oKATCPClientCallbackHandler.m_dFrequencyFs_Hz;
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

int32_t cKATCPServer::getNumberChannels_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    // This isn't terribly discerning code. If we had more modes we would need to do this better,
    // but it is mainly just for the automated test suite.
    if (m_oKATCPClientCallbackHandler.m_i32SizeOfFineFFT_nSamp == 0)
        return 1024;
    else
        return 4096;
}

int32_t cKATCPServer::getCoarseFFTShiftMask_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_i32CoarseFFTShiftMask;
}

double cKATCPServer::getDspGain_KATCPCallback(katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    return m_oKATCPClientCallbackHandler.m_dDspGain;
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
    //m_pInitialSensorDataThread.reset(new boost::thread(&cKATCPServer::initialSensorDataThreadFunction));
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
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

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

void cKATCPServer::cKATCPClientCallbackHandler::startRecording_callback(const string &strFileSuffix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void cKATCPServer::cKATCPClientCallbackHandler::stopRecording_callback()
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
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

void cKATCPServer::cKATCPClientCallbackHandler::frequencyFs_callback(double dFrequencyFs_Hz)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_dFrequencyFs_Hz = dFrequencyFs_Hz;
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

void cKATCPServer::cKATCPClientCallbackHandler::dspGain_callback(int64_t i64Timestamp_us, double dDspGain)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_dDspGain = dDspGain;
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


//Plagiarised from cKATCPClientBase
std::vector<std::string> cKATCPServer::tokeniseString(const std::string &strInputString, const std::string &strSeparators)
{
    //This funciton is not complete efficient due to extra memory copies of filling the std::vector
    //It will also be copied again on return.
    //It does simply the calling code and should be adequate in the context of most KATCP control clients.

    boost::char_separator<char> oSeparators(strSeparators.c_str());
    boost::tokenizer< boost::char_separator<char> > oTokens(strInputString, oSeparators);

    vector<string> vstrTokens;

    for(boost::tokenizer< boost::char_separator<char> >::iterator it = oTokens.begin(); it != oTokens.end(); ++it)
    {
        vstrTokens.push_back(*it);
        boost::trim(vstrTokens.back()); //Remove any possible whitespace etc from sides of token
    }

    return vstrTokens;
}
