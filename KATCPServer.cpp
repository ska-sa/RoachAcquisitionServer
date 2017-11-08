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
    add_version_katcp(m_pKATCPDispatch, const_cast<char*>("RoachAcquisitionServer"), 0, const_cast<char*>("0.1"), &oSSCompileTime.str()[0]);

    //Declare sensors
    //Station controller

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("stationControllerConnected"),
                                  const_cast<char*>("Is the RoachAcquisitionServer connected to the StationController's KATCP server"),
                                  const_cast<char*>("none"),
                                  &getIsStationControllerKATCPConnected, NULL, NULL);

    //Recording info.
    // TODO: figure out sensible min and max values for these.
    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingStartTime"),
                                  const_cast<char*>("Unix time at which the current recording started."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingStartTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingElapsedTime"),
                                  const_cast<char*>("Duration of current recording."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingElapsedTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingStopTime"),
                                  const_cast<char*>("Unix time at which the current recording is scheduled to stop."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingStopTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingRemainingTime"),
                                  const_cast<char*>("Time until the current recording is scheduled to stop."),
                                  const_cast<char*>("seconds"),
                                  &getRecordingRemainingTime_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingFileSize"),
                                  const_cast<char*>("Current size of file being recorded."),
                                  const_cast<char*>("bytes"),
                                  &getRecordingFileSize_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("recordingDiskSpace"),
                                  const_cast<char*>("Amount of disk space still available on SDB."),
                                  const_cast<char*>("bytes"),
                                  &getDiskSpace_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    /*
    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("requestedAntennaAz"),
                                 const_cast<char*>("requested antenna azimuth after the pointing model"),
                                 const_cast<char*>("degrees"),
                                 &getRequestedAntennaAz, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("requestedAntennaEl"),
                                 const_cast<char*>("requested antenna elevation after the pointing model"),
                                 const_cast<char*>("degrees"),
                                 &getRequestedAntennaEl, NULL, NULL, 0.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("actualAntennaAz"),
                                 const_cast<char*>("actual antenna azimuth after the pointing model"),
                                 const_cast<char*>("degrees"),
                                 &getActualAntennaAz, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("actualAntennaEl"),
                                 const_cast<char*>("actual antenna elevation after the pointing model"),
                                 const_cast<char*>("degrees"),
                                 &getActualAntennaEl, NULL, NULL, 0.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("actualSourceOffsetAz"),
                                 const_cast<char*>("actual azimuth offset from the source"),
                                 const_cast<char*>("degrees"),
                                 &getActualSourceOffsetAz, NULL, NULL, -90.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("actualSourceOffsetEl"),
                                 const_cast<char*>("actual elevation offset from the source"),
                                 const_cast<char*>("degrees"),
                                 &getActualSourceOffsetEl, NULL, NULL, -90.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("actualAntennaRA"),
                                 const_cast<char*>("actual antenna right ascension"),
                                 const_cast<char*>("degrees"),
                                 &getActualAntennaRA, NULL, NULL, -90.0, 90.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("actualAntennaDec"),
                                 const_cast<char*>("actual antenna declination"),
                                 const_cast<char*>("degrees"),
                                 &getActualAntennaDec, NULL, NULL, 0.0, 360.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("motorTorqueAzMaster"),
                                 const_cast<char*>("torque of master azimuth motor"),
                                 const_cast<char*>("mNm"),
                                 &getMotorTorqueAzMaster, NULL, NULL, -3600.0, 3600.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("motorTorqueAzSlave"),
                                 const_cast<char*>("torque of slave azimuth motor"),
                                 const_cast<char*>("mNm"),
                                 &getMotorTorqueAzSlave, NULL, NULL, -3600.0, 3600.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("motorTorqueElMaster"),
                                 const_cast<char*>("torque of master elevation motor"),
                                 const_cast<char*>("mNm"),
                                 &getMotorTorqueElMaster, NULL, NULL, -3600.0, 3600.0, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("motorTorqueElSlave"),
                                 const_cast<char*>("torque of slave elevation motor"),
                                 const_cast<char*>("mNm"),
                                 &getMotorTorqueElSlave, NULL, NULL, -3600.0, 3600.0, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("noiseDiodeSoftwareState"),
                                  const_cast<char*>("the state of the noise diode (on/off) when under FieldSystem control"),
                                  const_cast<char*>("none"),
                                  &getNoiseDiodeSoftwareState, NULL, NULL, -1, 1, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("noiseDiodeCurrent"),
                                 const_cast<char*>("the current draw through the noise diode(s)"),
                                 const_cast<char*>("A"),
                                 &getMotorTorqueElSlave, NULL, NULL, 0.0, 5.0, NULL);

    //TODO: Acceptable range values for the sensors before "warn" is shown. Very wide defaults right now.
    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("bandSelectedLCP"),
                                 const_cast<char*>("the centre frequency of RF mapped to final IF output channel 0"),
                                 const_cast<char*>("Hz"),
                                 &getFBandSelectedLCP, NULL, NULL, 0, 1, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("bandSelectedRCP"),
                                 const_cast<char*>("the centre frequency of RF mapped to final IF output channel 1"),
                                 const_cast<char*>("Hz"),
                                 &getBandSelectedRCP, NULL, NULL, 0, 1, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("frequencyLO0Chan0"),
                                 const_cast<char*>("the frequency for the first LO in RF input channel 0"),
                                 const_cast<char*>("Hz"),
                                 &getFrequencyLO0Chan0, NULL, NULL, 0, 7e9, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("frequencyLO0Chan1"),
                                 const_cast<char*>("the frequency for the first LO in RF input channel 1"),
                                 const_cast<char*>("Hz"),
                                 &getFrequencyLO0Chan1, NULL, NULL, 0, 7e9, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("frequencyLO1"),
                                 const_cast<char*>("the frequency for the final (IF) LO"),
                                 const_cast<char*>("Hz"),
                                 &getFrequencyLO1, NULL, NULL, 0, 7e9, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("receiverBandwidthChan0"),
                                 const_cast<char*>("the bandwidth available to the final IF output channel 0"),
                                 const_cast<char*>("Hz"),
                                 &getReceiverBandwidthChan0, NULL, NULL, 0, 7e9, NULL);

    register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                 const_cast<char*>("receiverBandwidthChan1"),
                                 const_cast<char*>("the bandwidth available to the final IF output channel 1"),
                                 const_cast<char*>("Hz"),
                                 &getReceiverBandwidthChan1, NULL, NULL, 0, 7e9, NULL);
    */

    //ROACH
    register_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachConnected"),
                                  const_cast<char*>("Is the RoachAcquisitionServer connected to the ROACH's KATCP server"),
                                  const_cast<char*>("none"),
                                  &getIsRoachKATCPConnected, NULL, NULL);

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachStokesEnabled"),
                                  const_cast<char*>("Is the ROACH spitting out LRQU data. (Otherwise LRPP)"),
                                  const_cast<char*>("none"),
                                  &getStokesEnabled, NULL, NULL);

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

    register_double_sensor_katcp(m_pKATCPDispatch, 0, const_cast<char*>("roachFrequencyFs"),
                                 const_cast<char*>("ADC sample rate"),
                                 const_cast<char*>("Hz"),
                                 &getFrequencyFs_KATCPCallback, NULL, NULL, 0.0, 1e9, NULL);

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
                                  const_cast<char*>("roachCoarseFFTShiftMask"),
                                  const_cast<char*>("Mask determining the scaling with the FFT stages"),
                                  const_cast<char*>("none"),
                                  &getCoarseFFTShiftMask_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

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

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachNoiseDiodeEnabled"),
                                  const_cast<char*>("Is the Roach's noise diode control enabled"),
                                  const_cast<char*>("none"),
                                  &getNoiseDiodeEnabled_KATCPCallback, NULL, NULL);

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0, const_cast<char*>("roachNoiseDiodeDutyCycleEnabled"),
                                  const_cast<char*>("Is the Roach generating a sample-clock synchronous duty cycle for the noise diode"),
                                  const_cast<char*>("none"),
                                  &getNoiseDiodeDutyCycleEnabled_KATCPCallback, NULL, NULL);

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

    register_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachEth10GbEUp"),
                                  const_cast<char*>("Is the relevant Roach 10GbE port for the current gateware link up"),
                                  const_cast<char*>("none"),
                                  &getEth10GbEUp_KATCPCallback, NULL, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachPPSCount"),
                                  const_cast<char*>("A count of the received PPS edges"),
                                  const_cast<char*>("none"),
                                  &getPPSCount_KATCPCallback, NULL, NULL, 0, INT_MAX, NULL);

    register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                                  const_cast<char*>("roachClockFrequency"),
                                  const_cast<char*>("The clock frequency of the FPGA"),
                                  const_cast<char*>("Hz"),
                                  &getClockFrequency_KATCPCallback, NULL, NULL, 0, 250000000, NULL);

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
                   &cKATCPServer::getCurrentFilename_KATCPCallback);
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

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?setRoachStokesEnabled"),
                   const_cast<char*>("Enable stokes calculation to output LRQU (otherwise LRPP)"),
                   &cKATCPServer::roachSetStokesEnabled_KATCPCallback);

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

    // Data from RF controller GUI.
    /*
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#set-valon-freq"), // This is the response from the infrastructure controller to the GUI with timestamp
                   const_cast<char*>("Push valon frequency to datafile (sent by RF control GUI)"),
                   &cKATCPServer::RFGUIReceiveValonFrequency_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#set-valon-freq:_FAILED"), // An error happened for some reason, need to log this.
                   const_cast<char*>("Push valon frequency to datafile (sent by RF control GUI)"),
                   &cKATCPServer::RFGUIReceiveValonFrequency_KATCPCallback);

    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#set-output"),
                   const_cast<char*>("Process attenuation and band selection (sent by RF control GUI)"),
                   &cKATCPServer::RFGUIReceiveSensorOutput_KATCPCallback);

    // Ignore most of the katcp messages coming from the RF control GUI.
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#version"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#build-state"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#log"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?set-output"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("!set-output"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?set-valon-freq"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("!set-valon-freq"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("#get-valon-freq"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?get-valon-freq"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("!get-valon-freq"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("?set-valon-options"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    register_katcp(m_pKATCPDispatch,
                   const_cast<char*>("!set-valon-options"),
                   const_cast<char*>("Ignored."),
                   &cKATCPServer::RFGUIIgnore_KATCPCallback);
    */

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

void cKATCPServer::initialSensorDataThreadFunction()
{
    //Record data which was stored before the file started recording (i.e. most recent sensor data).
    //General idea: If it's non-zero, we have received a value before the recording started.
    //Add a timestamp ten seconds ago (to ensure that it starts a bit before the file starts) and push to the file writer.

    //Sleep for a bit to ensure that the calling function is sufficiently dead. I noticed that if I tried to do this in the recordingStarted_callback then
    //it exited without acctually accomplishing its task. So it had to be in a new thread and wait for a bit to ensure that the HDF5 file was actually created
    //before it tries to record stuff. Weird.
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

    //Lock access for the rest of the function
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    if (m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan0_Hz != 0.0)
    {
        m_pFileWriter->recordFrequencyLO0Chan0((time(0) - 10)*1e6, m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan0_Hz, "nominal");
    }
    if (m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan1_Hz != 0.0)
    {
        m_pFileWriter->recordFrequencyLO0Chan1((time(0) - 10)*1e6, m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan1_Hz, "nominal");
    }
    if (m_oKATCPClientCallbackHandler.m_dFrequencyLO1_Hz != 0.0)
    {
        m_pFileWriter->recordFrequencyLO1((time(0) - 10)*1e6, m_oKATCPClientCallbackHandler.m_dFrequencyLO1_Hz, "nominal");
    }

    //Won't check for defaults here, because this should be zero as a default anyway.
    m_pFileWriter->recordFrequencySelectLCP((time(0) - 10)*1e6, m_oKATCPClientCallbackHandler.m_bBandSelectedLCP, "nominal");
    m_pFileWriter->recordFrequencySelectRCP((time(0) - 10)*1e6, m_oKATCPClientCallbackHandler.m_bBandSelectedRCP, "nominal");
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
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, const_cast<char*>("startRecording: No file recorder object set."));
        return KATCP_RESULT_FAIL;
    }

    if(i32ArgC > 4)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, const_cast<char*>("startRecording: Warning %i redundant argument(s) for stop recording, ignoring."), i32ArgC - 4);
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
        m_pStationControllerKATCPClient->unsubscribeSensorData();
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


double cKATCPServer::getRecordingStartTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return (m_pFileWriter->getRecordingStartTime_us())/1e6;
    }
    else
    {
        return 0.0;
    }
}


double cKATCPServer::getRecordingElapsedTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return (m_pFileWriter->getRecordedDuration_us())/1e6;
    }
    else
    {
        return 0.0;
    }
}


double cKATCPServer::getRecordingStopTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return (m_pFileWriter->getRecordingStopTime_us())/1e6;
    }
    else
    {
        return 0.0;
    }
}


double cKATCPServer::getRecordingRemainingTime_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    if(m_pFileWriter->isRecordingEnabled())
    {
        return (m_pFileWriter->getRecordingTimeLeft_us())/1e6;
    }
    else
    {
        return 0.0;
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

int32_t cKATCPServer::roachSetAccumulationLength_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
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

int32_t cKATCPServer::roachSetADC0Attenuation_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
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

//RF GUI information
int32_t cKATCPServer::RFGUIIgnore_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    //Most of the stuff that the RF GUI sends just needs to be ignored.
    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::RFGUIReceiveValonFrequency_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    //Sometimes this fails.
    if (string(arg_string_katcp(pKATCPDispatch, 0)) == "#set-valon-freq:_FAILED")
    {
        cout << "cKATCPServer::RFGUIReceiveValonFrequency_KATCPCallback(): Valon programming failed. Info to follow." << endl;
        for (int32_t i = 1; i < i32ArgC; i++)
        {
            cout << "cKATCPServer::RFGUIReceiveValonFrequency_KATCPCallback(): Received katcp message: " << string(arg_string_katcp(pKATCPDispatch, i)) << endl;
        }
    }
    else
    {
        double dTimestamp_s = strtod(arg_string_katcp(pKATCPDispatch, 1), NULL) / 1e3; // Timestamp is in milliseconds.
        vector<string> v_strValonInfo = tokeniseString(string(arg_string_katcp(pKATCPDispatch, i32ArgC - 1)), string(" "));
        // I read somewhere that boost's lexical_cast is slow.
        // But it's easy and it's in a separate thread from the critical stuff,
        // plus this will only happen on changes of frequency so I figure it's okay.
        uint32_t u32ValonNumber = boost::lexical_cast<int>(v_strValonInfo[0]);
        char chSynthLetter = boost::lexical_cast<char>(v_strValonInfo[1]);
        double dSynthFrequency_Hz = boost::lexical_cast<double>(v_strValonInfo[2]);
        string strUnit = v_strValonInfo[3];

        //This shouldn't be an issue, but just in case.
        if (strUnit == "Hz")
            ;
        else if (strUnit == "kHz")
            dSynthFrequency_Hz *= 1e3;
        else if (strUnit == "GHz")
            dSynthFrequency_Hz *= 1e9;
        else
            dSynthFrequency_Hz *= 1e6; // If it's something else, just assume that it's MHz. That seems to be the standard.

        cout << "cKATCPServer::RFGUIReceiveValonFrequency_KATCPCallback(): Received change in frequency: Valon ";
        switch (u32ValonNumber)
        {
            case 0: // Front-most valon.
            {
                cout << "0 synth ";
                switch (chSynthLetter)
                {
                    case 'a': // 5.0 GHz's oscillator
                        cout << "a: " << dSynthFrequency_Hz / 1e6 << " MHz";
                        m_oKATCPClientCallbackHandler.frequencyLO0Chan0_callback(dTimestamp_s * 1e6, dSynthFrequency_Hz, "nominal"); // Let's give this a try.
                        m_pFileWriter->recordFrequencyLO0Chan0(dTimestamp_s * 1e6, dSynthFrequency_Hz, "nominal"); // hardcoded to nominal for the time being.
                        break;
                    case 'b': // 6.7 GHz's oscillator
                        cout << "b: " << dSynthFrequency_Hz / 1e6 << " MHz";
                        m_oKATCPClientCallbackHandler.frequencyLO0Chan1_callback(dTimestamp_s * 1e6, dSynthFrequency_Hz, "nominal"); // Let's give this a try.
                        m_pFileWriter->recordFrequencyLO0Chan1(dTimestamp_s * 1e6, dSynthFrequency_Hz, "nominal"); // hardcoded to nominal for the time being.
                        break;
                    default: // This is if a problem has been encountered.
                        cout << "UNKNOWN!";
                        return KATCP_RESULT_INVALID;
                        break;
                }
            } break;

            case 1:
            {
                cout << "1 with synth ";
                switch (chSynthLetter)
                {
                    case 'a': // Final stage oscillator
                        cout << "a: " << dSynthFrequency_Hz / 1e6 << " MHz";
                        m_oKATCPClientCallbackHandler.frequencyLO1_callback(dTimestamp_s * 1e6, dSynthFrequency_Hz, "nominal");
                        m_pFileWriter->recordFrequencyLO1(dTimestamp_s * 1e6, dSynthFrequency_Hz, "nominal");
                        break;
                    case 'b':
                        cout << "b, but this is unused. ";
                    default:
                        cout << "UNKNOWN!";
                        return KATCP_RESULT_INVALID;
                        break;
                }
            } break;

            default:
            {
                cout << "Error! Unknown valon number.";
                return KATCP_RESULT_INVALID;
            } break;

        }
        cout << "." << endl;
    }

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::RFGUIReceiveSensorOutput_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC)
{
    double dTimestamp_s = strtod(arg_string_katcp(pKATCPDispatch, 1), NULL) / 1e3; // Timestamp is in milliseconds.
    string strSensorName = arg_string_katcp(pKATCPDispatch, 3);
    string strSensorValue = arg_string_katcp(pKATCPDispatch, 4);

    //cout << "cKATCPServer::RFGUIReceiveSensorOutput_KATCPCallback(): Sensor name: " << arg_string_katcp(pKATCPDispatch, 3);
    //cout << " and value " <<  arg_string_katcp(pKATCPDispatch, 4) << endl;
    cout << "cKATCPServer::RFGUIReceiveSensorOutput_KATCPCallback(): Received " << strSensorValue << " change in ";

    if (strSensorName == "RFC.LcpFreqSel")
    {
        string strBand = (strSensorValue == "ON")?"6.7":"5.0";
        cout << "LCP band selection. Now using " << strBand << " GHz." << endl;
        m_oKATCPClientCallbackHandler.bandSelectLCP_callback(dTimestamp_s*1e6, (strSensorValue == "ON")?true:false, "nominal");
        m_pFileWriter->recordFrequencySelectLCP(dTimestamp_s * 1e6, (strSensorValue == "ON")?true:false, "nominal");
    }
    else if (strSensorName == "RFC.RcpFreqSel")
    {
        string strBand = (strSensorValue == "ON")?"6.7":"5.0";
        cout << "RCP band selection. Now using " << strBand << " GHz." << endl;
        m_oKATCPClientCallbackHandler.bandSelectRCP_callback(dTimestamp_s*1e6, (strSensorValue == "ON")?true:false, "nominal");
        m_pFileWriter->recordFrequencySelectRCP(dTimestamp_s * 1e6, (strSensorValue == "ON")?true:false, "nominal");
    }
    else if (strSensorName == "RFC.LcpAttenuation_0")
    {

    }
    else if (strSensorName == "RFC.LcpAttenuation_1")
    {

    }
    else if (strSensorName == "RFC.LcpAttenuation_2")
    {

    }
    else if (strSensorName == "RFC.LcpAttenuation_3")
    {

    }
    else if (strSensorName == "RFC.LcpAttenuation_4")
    {

    }
    else if (strSensorName == "RFC.LcpAttenuation_5")
    {

    }
    else if (strSensorName == "RFC.RcpAttenuation_0")
    {

    }
    else if (strSensorName == "RFC.RcpAttenuation_1")
    {

    }
    else if (strSensorName == "RFC.RcpAttenuation_2")
    {

    }
    else if (strSensorName == "RFC.RcpAttenuation_3")
    {

    }
    else if (strSensorName == "RFC.RcpAttenuation_4")
    {

    }
    else if (strSensorName == "RFC.RcpAttenuation_5")
    {

    }
    else
    {
        //ignore.
    }



    // Included here in case I decide I need them again:
    /*for (int32_t i = 0; i < i32ArgC; i++)
    {
        cout << "cKATCPServer::RFGUIReceiveSensorOutput_KATCPCallback(): Received katcp message: " << string(arg_string_katcp(pKATCPDispatch, i)) << endl;
    }*/
    return KATCP_RESULT_OK;

    //RFC.LcpFreqSel
    //RFC.RcpFreqSel
    //off is 5GHz
    //on is 6.7 GHz
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
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dRequestedAntennaAz_deg;
}

double cKATCPServer::getRequestedAntennaEl(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dRequestedAntennaEl_deg;
}

double cKATCPServer::getActualAntennaAz(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaAz_deg;
}

double cKATCPServer::getActualAntennaEl(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaEl_deg;
}

double cKATCPServer::getActualSourceOffsetAz(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dActualSourceOffsetAz_deg;
}

double cKATCPServer::getActualSourceOffsetEl(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dActualSourceOffsetEl_deg;
}

double cKATCPServer::getActualAntennaRA(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaRA_deg;
}

double cKATCPServer::getActualAntennaDec(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dActualAntennaDec_deg;
}

//char* cKATCPServer::getAntennaStatus(struct katcp_dispatch *pD, katcp_acquire *pA)
//{
//    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

//    return m_oKATCPClientCallbackHandler.m_strAntennaStatus.c_str();
//}

double cKATCPServer::getMotorTorqueAzMaster(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueAzMaster_mNm;
}

double cKATCPServer::getMotorTorqueAzSlave(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueAzSlave_mNm;
}

double cKATCPServer::getMotorTorqueElMaster(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueElMaster_mNm;
}

double cKATCPServer::getMotorTorqueElSlave(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dMotorTorqueElSlave_mNm;
}

int32_t cKATCPServer::getNoiseDiodeSoftwareState(katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_i32NoiseDiodeSoftwareState;
}

//char* cKATCPServer::getNoiseDiodeSource(struct katcp_dispatch *pD, struct katcp_acquire *pA)
//{
//    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oStationControllerMutex);

//    return m_oKATCPClientCallbackHandler.m_strNoiseDiodeSource.c_str();
//}

double cKATCPServer::getNoiseDiodeCurrent(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dNoiseDiodeCurrent_A;
}

double cKATCPServer::getFBandSelectedLCP(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_bBandSelectedLCP;
}

double cKATCPServer::getBandSelectedRCP(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_bBandSelectedRCP;
}

double cKATCPServer::getFrequencyLO0Chan0(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan0_Hz;
}

double cKATCPServer::getFrequencyLO0Chan1(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyLO0Chan1_Hz;
}

double cKATCPServer::getFrequencyLO1(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dFrequencyLO1_Hz;
}

double cKATCPServer::getReceiverBandwidthChan0(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dReceiverBandwidthChan0_Hz;
}

double cKATCPServer::getReceiverBandwidthChan1(struct katcp_dispatch *pD, katcp_acquire *pA)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    return m_oKATCPClientCallbackHandler.m_dReceiverBandwidthChan1_Hz;
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
    m_pInitialSensorDataThread.reset(new boost::thread(&cKATCPServer::initialSensorDataThreadFunction));
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

void cKATCPServer::cKATCPClientCallbackHandler::startRecording_callback(const string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void cKATCPServer::cKATCPClientCallbackHandler::stopRecording_callback()
{
    //Not used. Note, the HDF5FileWriter class is also a callback handler for KATCPClient so it get this callback to too and reacts to it there.
}

void  cKATCPServer::cKATCPClientCallbackHandler::requestedAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dRequestedAntennaAz_deg = dAzimuth_deg;
}

void  cKATCPServer::cKATCPClientCallbackHandler::requestedAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatusf)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dRequestedAntennaEl_deg = dElevation_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dActualAntennaAz_deg = dAzimuth_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dActualAntennaEl_deg = dElevation_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualSourceOffsetAz_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dActualSourceOffsetAz_deg = dAzimuthOffset_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualSourceOffsetEl_callback(int64_t i64Timestamp_us, double dElevationOffset_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dActualSourceOffsetEl_deg = dElevationOffset_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaRA_callback(int64_t i64Timestamp_us, double dRighAscension_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dActualAntennaRA_deg = dRighAscension_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::actualAntennaDec_callback(int64_t i64Timestamp_us, double dDeclination_deg, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dActualAntennaDec_deg = dDeclination_deg;
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaStatus_callback(int64_t i64Timestamp_us, const string &strAntennaStatus, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_strAntennaStatus = strStatus;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueAzMaster_callback(int64_t i64Timestamp_us, double dAzMaster_mNm, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dMotorTorqueAzMaster_mNm = dAzMaster_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueAzSlave_callback(int64_t i64Timestamp_us, double dAzSlave_mNm, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dMotorTorqueAzSlave_mNm = dAzSlave_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueElMaster_callback(int64_t i64Timestamp_us, double dElMaster_mNm, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dMotorTorqueElMaster_mNm = dElMaster_mNm;
}

void cKATCPServer::cKATCPClientCallbackHandler::motorTorqueElSlave_callback(int64_t i64Timestamp_us, double dElSlave_mNm, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dMotorTorqueAzSlave_mNm = dElSlave_mNm;
}

/* TODO: This marked for removal.
void cKATCPServer::cKATCPClientCallbackHandler::appliedPointingModel_callback(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    //Todo
    //JNS: Is there anything TODO here?
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaName_callback(const string &strAntennaName)
{
    //TODO
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaDiameter_callback(const std::string &strAntennaDiameter)
{
    //TODO
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaBeamwidth_callback(const std::string &strAntennaBeamwidth)
{
    //TODO
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaLongitude_callback(const std::string &strAntennaLongitude)
{
    //TODO
}

void cKATCPServer::cKATCPClientCallbackHandler::antennaLatitude_callback(const std::string &strAntennaLatitude)
{
    //TODO
}
*/

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_i32NoiseDiodeSoftwareState = i32NoiseDiodeState;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeSource_callback(int64_t i64Timestamp_us, const string &strNoiseDiodeSource, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_strNoiseDiodeSource = strNoiseDiodeSource;
}

void cKATCPServer::cKATCPClientCallbackHandler::noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dNoiseDiodeCurrent_A = dNoiseDiodeCurrent_A;
}

void cKATCPServer::cKATCPClientCallbackHandler::sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg)
{
    //Todo
}

void cKATCPServer::cKATCPClientCallbackHandler::bandSelectLCP_callback(int64_t i64Timestamp_us, bool bBandSelected, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_bBandSelectedLCP = bBandSelected;
}

void cKATCPServer::cKATCPClientCallbackHandler::bandSelectRCP_callback(int64_t i64Timestamp_us, bool bBandSelected, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_bBandSelectedRCP = bBandSelected;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLO0Chan0_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_Hz, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dFrequencyLO0Chan0_Hz = dFrequencyLO0Chan0_Hz;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLO0Chan1_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_Hz, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dFrequencyLO0Chan1_Hz = dFrequencyLO0Chan1_Hz;
}

void cKATCPServer::cKATCPClientCallbackHandler::frequencyLO1_callback(int64_t i64Timestamp_us, double dFrequencyLO1_Hz, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dFrequencyLO1_Hz = dFrequencyLO1_Hz;
}

void cKATCPServer::cKATCPClientCallbackHandler::receiverBandwidthChan0_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_Hz, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dReceiverBandwidthChan0_Hz = dReceiverBandwidthChan0_Hz;
}

void cKATCPServer::cKATCPClientCallbackHandler::receiverBandwidthChan1_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_Hz, const string &strStatus)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oSensorDataMutex);

    m_dReceiverBandwidthChan1_Hz = dReceiverBandwidthChan1_Hz;
}

/*
void cKATCPServer::cKATCPClientCallbackHandler::stokesEnabled_callback(int64_t i64Timestamp_us, bool bEnabled)
{
    boost::unique_lock<boost::mutex> oLock(m_oKATCPClientCallbackHandler.m_oRoachMutex);

    m_bStokesEnabled = bEnabled;
}*/

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
