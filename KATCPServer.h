#ifndef KATCTP_SERVER_H
#define KATCTP_SERVER_H

//System includes
#include <string>
#include <vector>

#ifdef _WIN32
#include <stdint.h>

#ifndef int64_t
typedef __int64 int64_t;
#endif

#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#else
#include <inttypes.h>
#endif

//Library includes
extern "C" {
#include <katcp.h>
}

#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#endif

//Local includes
#include "HDF5FileWriter.h"
#include "RoachKATCPClient.h"
#include "StationControllerKATCPClient.h"

//This class is pure static to allow callback functions to comply with the C callback format required by the KATCP library.
//This means only a single instance of this class can exist but this should suite just about every use case.
//It seems unlikely that an application would require multiple servers. In this case the class can, however, be cloned.

//NB use of this class requires libkatcp compiled with -DKATCP_USE_FLOATS to enable double based sensors.
//This is on by default in Jan 2016 releases in Makefile.inc of ska-sa/katcp_devel library

class cKATCPServer
{
public:
    //cKATCPServer is pure static and therefore cannot derive the non-static callback interfaces

    //So define a new non-static derived classes here as callback handlers and create instances as
    //static members of the cKATCPServer class.

    struct cHDF5FileWriterCallbackHandler : public cHDF5FileWriter::cCallbackInterface
    {
        //Callback functions called from HDF5FileWriter

        void                                                recordingStarted_callback();
        void                                                recordingStopped_callback();
    };

    struct cKATCPClientCallbackHandler : public cRoachKATCPClient::cCallbackInterface, public cStationControllerKATCPClient::cCallbackInterface
    {
        //Locals for metadata sensor values
        bool                                                m_bStationControllerKATCPConnected;
        boost::mutex                                        m_oStationControllerMutex;
        std::string                                         m_strStationControllerAddress;
        double                                              m_dRequestedAntennaAz_deg;
        double                                              m_dRequestedAntennaEl_deg;
        double                                              m_dActualAntennaAz_deg;
        double                                              m_dActualAntennaEl_deg;
        double                                              m_dActualSourceOffsetAz_deg;
        double                                              m_dActualSourceOffsetEl_deg;
        double                                              m_dActualAntennaRA_deg;
        double                                              m_dActualAntennaDec_deg;
        std::string                                         m_strAntennaStatus;
        double                                              m_dMotorTorqueAzMaster_mNm;
        double                                              m_dMotorTorqueAzSlave_mNm;
        double                                              m_dMotorTorqueElMaster_mNm;
        double                                              m_dMotorTorqueElSlave_mNm;
        int32_t                                             m_i32NoiseDiodeSoftwareState;
        std::string                                         m_strNoiseDiodeSource;
        double                                              m_dNoiseDiodeCurrent_A;
        bool                                                m_bBandSelectedLCP;
        bool                                                m_bBandSelectedRCP;
        double                                              m_dFrequencyLO0Chan0_Hz;
        double                                              m_dFrequencyLO0Chan1_Hz;
        double                                              m_dFrequencyLO1_Hz;
        double                                              m_dReceiverBandwidthChan0_Hz;
        double                                              m_dReceiverBandwidthChan1_Hz;
        boost::mutex                                        m_oSensorDataMutex;
        //Given that there is only 1 read and 1 write thread a single mutex for all variables should suffice.

        //Callback functions called from the Station controller KATCPClient

        void                                                connected_callback(bool bConnected, const std::string &strHostAddress, uint16_t u16Port, const std::string &strDescription);

        //File recording control
        void                                                startRecording_callback(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
        void                                                stopRecording_callback();

        //Antenna values
        void                                                requestedAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
        void                                                requestedAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);
        void                                                actualAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
        void                                                actualAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);
        void                                                actualSourceOffsetAz_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg, const std::string &strStatus);
        void                                                actualSourceOffsetEl_callback(int64_t i64Timestamp_us, double dElevationOffset_deg, const std::string &strStatus);
        void                                                actualAntennaRA_callback(int64_t i64Timestamp_us, double dRighAscension_deg, const std::string &strStatus);
        void                                                actualAntennaDec_callback(int64_t i64Timestamp_us, double dDeclination_deg, const std::string &strStatus);

        void                                                antennaStatus_callback(int64_t i64Timestamp_us, const std::string &strAntennaStatus, const std::string &strStatus);
        void                                                motorTorqueAzMaster_callback(int64_t i64Timestamp_us, double dAzMaster_mNm, const std::string &strStatus);
        void                                                motorTorqueAzSlave_callback(int64_t i64Timestamp_us, double dAzSlave_mNm, const std::string &strStatus);
        void                                                motorTorqueElMaster_callback(int64_t i64Timestamp_us, double dElMaster_mNm, const std::string &strStatus);
        void                                                motorTorqueElSlave_callback(int64_t i64Timestamp_us, double dElSlave_mNm, const std::string &strStatus);
/* TODO: Antenna config info marked for removal.
        void                                                appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);
        void                                                antennaName_callback(const std::string &strAntennaName);
        void                                                antennaDiameter_callback(const std::string &strAntennaDiameter);
        void                                                antennaBeamwidth_callback(const std::string &strAntennaBeamwidth);
        void                                                antennaLongitude_callback(const std::string &strAntennaLongitude);
        void                                                antennaLatitude_callback(const std::string &strAntennaLatitude);
*/
        //Noise diode values
        void                                                noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const std::string &strStatus);
        void                                                noiseDiodeSource_callback(int64_t i64Timestamp_us, const std::string &strNoiseDiodeSource, const std::string &strStatus);
        void                                                noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A, const std::string &strStatus);

        //Global experiment values
        void                                                sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);


        //RF values
        void                                                bandSelectLCP_callback(int64_t i64Timestamp_us, bool bBandSelected, const std::string &strStatus);
        void                                                bandSelectRCP_callback(int64_t i64Timestamp_us, bool bBandSelected, const std::string &strStatus);
        void                                                frequencyLO0Chan0_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_Hz, const std::string &strStatus);
        void                                                frequencyLO0Chan1_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_Hz, const std::string &strStatus);
        void                                                frequencyLO1_callback(int64_t i64Timestamp_us, double dFrequencyLO1_Hz, const std::string &strStatus);
        void                                                receiverBandwidthChan0_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_Hz, const std::string &strStatus);
        void                                                receiverBandwidthChan1_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_Hz, const std::string &strStatus);


        //Locals for ROACH values
        bool                                                m_bRoachKATCPConnected;
        std::string                                         m_strRoachAddress;
        bool                                                m_bStokesEnabled;
        int32_t                                             m_i32AccumulationLength_nFrames;
        int32_t                                             m_i32CoarseChannelSelection;
        double                                              m_dFrequencyFs_Hz;
        int32_t                                             m_i32SizeOfCoarseFFT_nSamp;
        int32_t                                             m_i32SizeOfFineFFT_nSamp;
        int32_t                                             m_i32CoarseFFTShiftMask;
        double                                              m_dADCAttenuationChan0_dB;
        double                                              m_dADCAttenuationChan1_dB;
        bool                                                m_bNoiseDiodeEnabled;
        bool                                                m_bNoiseDiodeDutyCycleEnabled;
        int32_t                                             m_i32NoiseDiodeDutyCycleOnDuration_nAccums;
        int32_t                                             m_i32NoiseDiodeDutyCycleOffDuration_nAccums;
        int32_t                                             m_i32OverflowRegs;
        bool                                                m_bEth10GbEUp;
        int32_t                                             m_i32PPSCount;
        int32_t                                             m_i32ClockFrequency;
        boost::mutex                                        m_oRoachMutex;

        //Callback functions from the Roach KATCP client
        void                                                stokesEnabled_callback(int64_t i64Timestamp_us, bool bEnabled);
        void                                                accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames);
        void                                                coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
        void                                                frequencyFs_callback(double dFrequencyFs_Hz);
        void                                                sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp);
        void                                                sizeOfFineFFT_callback(uint32_t u32FineFFTSize_nSamp);
        void                                                coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
        void                                                attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB);
        void                                                attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB);
        void                                                noiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoideDiodeEnabled);
        void                                                noiseDiodeDutyCycleEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeDutyCyleEnabled);
        void                                                noiseDiodeDutyCycleOnDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums);
        void                                                noiseDiodeDutyCycleOffDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums);
        void                                                overflowsRegs_callback(int64_t i64Timestamp_us, uint32_t u32OverflowRegs);
        void                                                eth10GbEUp_callback(int64_t i64Timestamp_us, bool bEth10GbEUp);
        void                                                ppsCount_callback(int64_t i64Timestamp_us, uint32_t u32PPSCount);
        void                                                clockFrequency_callback(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz);
    };

    cKATCPServer(const std::string &strListenInterface = std::string("0.0.0.0"), uint16_t u16Port = 7147, uint32_t u32MaxClients = 5, const string &strRoachGatewareDirectory = std::string(""));
    ~cKATCPServer();

    static void                                             setFileWriter(boost::shared_ptr<cHDF5FileWriter> pFileWriter);
    static void                                             setRoachKATCPClient(boost::shared_ptr<cRoachKATCPClient> pRoachKATCPClient);
    static void                                             setStationControllerKATCPClient(boost::shared_ptr<cStationControllerKATCPClient> pKATCPClient);

    static void                                             startServer(const std::string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients);
    static void                                             stopServer();

protected:
    //Callback handlers (Need to have new actual objects here as we cannot derive the callback handler classes with this class as it is static.)
    static cHDF5FileWriterCallbackHandler                   m_oHDF5FileWriterCallBackHandler; //TODO: This needs to have a few callbacks added. I think.
    static cKATCPClientCallbackHandler                      m_oKATCPClientCallbackHandler;

    static void                                             serverThreadFunction();
    static void                                             initialSensorDataThreadFunction();

    static struct katcp_dispatch                            *m_pKATCPDispatch;

    //KATCTP server callbacks
    //Recording functions:
    static int32_t                                          startRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          stopRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          getRecordingInfo_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          getRecordingStatus_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    //Controlling of Roach
    static int32_t                                          getRoachGatewareList_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachProgram_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetStokesEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetAccumulationLength_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetCoarseChannelSelect_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetCoarseFFTShiftMask_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetADC0Attenuation_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetADC1Attenuation_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetNoiseDiodeEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetNoiseDiodeDutyCycleEnabled_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetNoiseDiodeDutyCycleOnDuration_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          roachSetNoiseDiodeDutyCycleOffDuration_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    //RF GUI information (These should be a temporary solution)
    static int32_t                                          RFGUIIgnore_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          RFGUIReceiveValonFrequency_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          RFGUIReceiveSensorOutput_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);

    //Sensor get functions (Use by KATCP to read sensor values. Requires thread safe access where necessary)
    //Station controller values:
    static int                                              getIsStationControllerKATCPConnected(struct katcp_dispatch *pD, katcp_acquire *pA);

    //Antenna
    static double                                           getRequestedAntennaAz(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getRequestedAntennaEl(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getActualAntennaAz(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getActualAntennaEl(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getActualSourceOffsetAz(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getActualSourceOffsetEl(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getActualAntennaRA(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getActualAntennaDec(struct katcp_dispatch *pD, katcp_acquire *pA);

    //static char*                                            getAntennaStatus(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getMotorTorqueAzMaster(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getMotorTorqueAzSlave(struct katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getMotorTorqueElMaster(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getMotorTorqueElSlave(struct katcp_dispatch *pD, katcp_acquire *pA);
    //void                                                  getAppliedPointingModel(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);

    //Noise diode
    static int32_t                                          getNoiseDiodeSoftwareState(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    //static char*                                            getNoiseDiodeSource(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getNoiseDiodeCurrent(struct katcp_dispatch *pD, katcp_acquire *pA);

    //Global experiment
    void                                                    getSourceSelection(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);

    //RFE
    static double                                           getFBandSelectedLCP(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getBandSelectedRCP(katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getFrequencyLO0Chan0(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getFrequencyLO0Chan1(katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getFrequencyLO1(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getReceiverBandwidthChan0(katcp_dispatch *pD, katcp_acquire *pA);
    static double                                           getReceiverBandwidthChan1(struct katcp_dispatch *pD, struct katcp_acquire *pA);

    //Roach values:
    static int32_t                                          getIsRoachKATCPConnected(struct katcp_dispatch *pD, katcp_acquire *pA);
    static int32_t                                          getStokesEnabled(struct katcp_dispatch *pD, katcp_acquire *pA);
    static int32_t                                          getAccumulationLength_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getCoarseChannelSelect_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getFrequencyFs_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getSizeOfCoarseFFT_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getSizeOfFineFFT_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getCoarseFFTShiftMask_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getADCAttenuationChan0_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static double                                           getADCAttenuationChan1_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getNoiseDiodeEnabled_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getNoiseDiodeDutyCycleEnabled_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getNoiseDiodeDutyCycleOnDuration_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getNoiseDiodeDutyCycleOffDuration_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getOverflowsRegs_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getEth10GbEUp_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getPPSCount_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
    static int32_t                                          getClockFrequency_KATCPCallback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

    static boost::shared_ptr<cHDF5FileWriter>               m_pFileWriter;
    static boost::shared_ptr<cRoachKATCPClient>             m_pRoachKATCPClient;
    static boost::shared_ptr<cStationControllerKATCPClient> m_pStationControllerKATCPClient;

    //Threads
    static boost::scoped_ptr<boost::thread>                 m_pKATCPThread;
    static boost::shared_ptr<boost::thread>                 m_pInitialSensorDataThread;

    //Members description operation state
    static std::string                                      m_strListenInterface;
    static uint16_t                                         m_u16Port;
    static uint32_t                                         m_u32MaxClients;
    static std::string                                      m_strRoachGatewareDirectory;

    //Tokenize function to split multi-word dispatches.
    static std::vector<std::string>                         tokeniseString(const std::string &strInputString, const std::string &strSeparators);

};

#endif // KATCTP_SERVER_H
