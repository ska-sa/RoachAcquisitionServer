#ifndef HDF5_FILE_WRITER_H
#define HDF5_FILE_WRITER_H

//System includes
#include <string>

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
#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#endif

//Local includes
#include "AVNAppLibs/SocketStreamers/UDPReceiver/UDPReceiver.h"
#include "AVNDataTypes/SpectrometerDataStream/SpectrometerDataStreamInterpreter.h"
#include "AVNDataTypes/SpectrometerDataStream/SpectrometerHDF5OutputFile.h"
#include "StationControllerKATCPClient.h"
#include "RoachKATCPClient.h"

class cHDF5FileWriter : public cSpectrometerDataStreamInterpreter::cCallbackInterface,
        public cUDPReceiver::cDataCallbackInterface,
        public cStationControllerKATCPClient::cCallbackInterface,
        public cRoachKATCPClient::cCallbackInterface
{
    //cSpectrometerDataStreamInterpreter actually implements cUDPReceiver::cCallbackInterface as well and could therefore be connected directly to the UDPReceiver to get
    //the datastream from the Roach. However in the design of the class hierachy it made more sense that this HDF5Writer be the class to interface with the UDPReceiver and
    //have an instance of cSpectrometerDataStreamInterpreter as a member. This class therefore relays the offloadData_callback() call to the UDPReceiver member by regular
    //function call to have it process the data stream. It then implements cSpectrometerDataStreamInterpreter's callback interface to get the interpretted data back before
    //writing this to HDF5 file. This behaviour is little bit redundant but it offers the most flexibility if any of these classes are to be reused elsewhere.

    //The entire class hierachy will probably need to be reconsidered in time.

    typedef struct cInitialValueSet
    {
        // Antenna-space requested azim - cTimestampedDouble
        int64_t m_i64TSAcsRequestedAzim_us;
        double  m_dVAcsRequestedAzim_deg;
        char    m_chaAcsRequestedAzimStatus[7];

        // Antenna-space requested elev - cTimestampedDouble
        int64_t m_i64TSAcsRequestedElev_us;
        double  m_dVAcsRequestedElev_deg;
        char    m_chaAcsRequestedElevStatus[7];

        // Antenna-space desired azim - cTimestampedDouble
        int64_t m_i64TSAcsDesiredAzim_us;
        double  m_dVAcsDesiredAzim_deg;
        char    m_chaAcsDesiredAzimStatus[7];

        // Antenna-space desired elev - cTimestampedDouble
        int64_t m_i64TSAcsDesiredElev_us;
        double  m_dVAcsDesiredElev_deg;
        char    m_chaAcsDesiredElevStatus[7];

        // Antenna-space actual azim - cTimestampedDouble
        int64_t m_i64TSAcsActualAzim_us;
        double  m_dVAcsActualAzim_deg;
        char    m_chaAcsActualAzimStatus[7];

        // Antenna-space actual elev - cTimestampedDouble
        int64_t m_i64TSAcsActualElev_us;
        double  m_dVAcsActualElev_deg;
        char    m_chaAcsActualElevStatus[7];

        // Sky-space requested azim - cTimestampedDouble
        int64_t m_i64TSSkyRequestedAzim_us;
        double  m_dVSkyRequestedAzim_deg;
        char    m_chaSkyRequestedAzimStatus[7];

        // Sky-space requested elev - cTimestampedDouble
        int64_t m_i64TSSkyRequestedElev_us;
        double  m_dVSkyRequestedElev_deg;
        char    m_chaSkyRequestedElevStatus[7];

        // Sky-space desired azim - cTimestampedDouble
        int64_t m_i64TSSkyDesiredAzim_us;
        double  m_dVSkyDesiredAzim_deg;
        char    m_chaSkyDesiredAzimStatus[7];

        // Sky-space desired elev - cTimestampedDouble
        int64_t m_i64TSSkyDesiredElev_us;
        double  m_dVSkyDesiredElev_deg;
        char    m_chaSkyDesiredElevStatus[7];

        // Sky-space actual azim - cTimestampedDouble
        int64_t m_i64TSSkyActualAzim_us;
        double  m_dVSkyActualAzim_deg;
        char    m_chaSkyActualAzimStatus[7];

        // Sky-space actual elev - cTimestampedDouble
        int64_t m_i64TSSkyActualElev_us;
        double  m_dVSkyActualElev_deg;
        char    m_chaSkyActualElevStatus[7];

        double  m_aPointingModel[30];

        int64_t m_i64TSAntennaStatus_us;
        char    m_chaVAntennaStatus[16];
        char    m_chaAntennaStatusStatus[7];

        // Receiver chain sky frequency - 5GHz band - cTimestampedDouble
        int64_t m_i64TSReceiverSkyFreq5GHz_us;
        double  m_dVReceiverSkyFreq5GHz_Hz;
        char    m_chaReceiverSkyFreq5GHzStatus[7];

        // Receiver chain sky frequency - 6_7GHz band - cTimestampedDouble
        int64_t m_i64TSReceiverSkyFreq6_7GHz_us;
        double  m_dVReceiverSkyFreq6_7GHz_Hz;
        char    m_chaReceiverSkyFreq6_7GHzStatus[7];

        // Receiver chain LCP attenuation - cTimestampedDouble
        int64_t m_i64TSReceiverLcpAtten_us;
        double  m_dVReceiverLcpAtten_dB;
        char    m_chaReceiverLcpAttenStatus[7];

        // Receiver chain RCP attenuation - cTimestampedDouble
        int64_t m_i64TSReceiverRcpAtten_us;
        double  m_dVReceiverRcpAtten_dB;
        char    m_chaReceiverRcpAttenStatus[7];

        // Receiver chain LCP Band Select - cTimestampedChar
        int64_t m_i64TSBandSelectLcp_us;
        char    m_chVBandSelectLcp;
        char    m_chaBandSelectLcpStatus[7];

        // Receiver chain RCP Band Select - cTimestampedChar
        int64_t m_i64TSBandSelectRcp_us;
        char    m_chVBandSelectRcp;
        char    m_chaBandSelectRcpStatus[7];

        // Noise diode input source - cNoiseDiodeSource
        int64_t m_i64TSNoiseDiodeInputSource_us;
        char    m_chaVNoiseDiodeInputSource[8];
        char    m_chaNoiseDiodeInputSourceStatus[7];

        // Noise diode enabled - cTimestampedBool
        int64_t m_i64TSNoiseDiodeEnabled_us;
        bool    m_bVNoiseDiodeEnabled;
        char    m_chaNoiseDiodeEnabledStatus[7];

        // Noise diode select - cTimestampedInt
        int64_t  m_i64TSNoiseDiodeSelect_us;
        int32_t  m_i32VNoiseDiodeSelect;
        char     m_chaNoiseDiodeSelectStatus[7];

        // Noise diode pwm mark - cTimestampedInt
        int64_t  m_i64TSNoiseDiodePWMMark_us;
        int32_t  m_i32VNoiseDiodePWMMark;
        char     m_chaNoiseDiodePWMMarkStatus[7];

        // Noise diode pwm frequency - cTimestampedDouble
        int64_t  m_i64TSNoiseDiodePWMFrequency_us;
        double   m_dVNoiseDiodePWMFrequency_Hz;
        char     m_chaNoiseDiodePWMFrequencyStatus[7];

        // Wind speed - cTimestampedDouble
        int64_t m_i64TSWindSpeed_us;
        double  m_dVWindSpeed_mps;
        char    m_chaWindSpeedStatus[7];

        // Wind direction - cTimestampedDouble
        int64_t m_i64TSWindDirection_us;
        double  m_dVWindDirection_deg;
        char    m_chaWindDirectionStatus[7];

        // Air temperature - cTimestampedDouble
        int64_t m_i64TSTemperature_us;
        double  m_dVTemperature_degC;
        char    m_chaTemperatureStatus[7];

        // Absolute pressure - cTimestampedDouble
        int64_t m_i64TSAbsolutePressure_us;
        double  m_dVAbsolutePressure_mbar;
        char    m_chaAbsolutePressureStatus[7];

        // Relative humidity - cTimestampedDouble
        int64_t m_i64TSRelativeHumidity_us;
        double  m_dVRelativeHumidity_percent;
        char    m_chaRelativeHumidityStatus[7];

        // accumulationLength - u32
        int64_t  m_i64TSAccumulationLength_us;
        uint32_t m_u32VAccumulationLength_frames;

        // coarseChannelSelect - u32
        int64_t  m_i64TSCoarseChannelSelect_us;
        uint32_t m_u32VCoarseChannelSelect;

        // coarseFFTShiftMask - u32
        int64_t  m_i64TSCoarseFFTShiftMask_us;
        uint32_t m_u32VCoarseFFTShiftMask;

        // dspGain - d
        int64_t  m_i64TSDspGain_us;
        double   m_dVDspGain;

        // attenuationADCChan0
        int64_t  m_i64TSAttenuationADCChan0_us;
        double   m_dVAttenuationADCChan0_dB;

        // attenuationADCChan1
        int64_t  m_i64TSAttenuationADCChan1_us;
        double   m_dVAttenuationADCChan1_dB;

    } cInitialValueSet;

public:
    class cCallbackInterface
    {
    public:
        virtual void                                    recordingStarted_callback() = 0;
        virtual void                                    recordingStopped_callback() = 0;
    };

    cHDF5FileWriter(const std::string &strRecordingDirectory, uint32_t u32FileSizeLimit_MB);
    ~cHDF5FileWriter();

    void                                                startRecording(const std::string &strFilenameSuffix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                                stopRecording();
    void                                                setRecordingDirectory(const std::string &strRecordingDirectory);

    //Public thread safe accessors
    bool                                                isRecordingEnabled();

    int64_t                                             getRecordingStartTime_us();
    int64_t                                             getRecordedDuration_us();
    int64_t                                             getRecordingStopTime_us();
    int64_t                                             getRecordingTimeLeft_us();

    std::string                                         getFilename();
    std::string                                         getRecordingDirectory();
    uint64_t                                            getCurrentFileSize_B();
    double                                              getCurrentFileSize_MB();

    //Re-implemented callback functions
    //Data is pushed into this function by the SpectrometerDataStreamInterpreter when a complete data frame is ready
    void                                                getNextFrame_callback(const std::vector<int> &vi32Chan0, const std::vector<int> &vi32Chan1,
                                                                             const std::vector<int> &vi32Chan2, std::vector<int> &vi32Chan3,
                                                                             const cSpectrometerHeader &oHeader);

    void                                                offloadData_callback(char* cpData, uint32_t u32Size_B);

    //Callbacks implement for KATCPClient callback interface. These pass information to be stored to HDF5 file:
    void                                                connected_callback(bool bConnected, const std::string &strHostAddress, uint16_t u16Port, const std::string &strDescription);

    //File recording control
    void                                                startRecording_callback(const std::string &strFileSuffix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                                stopRecording_callback();


    //Antenna values
    void                                                acsRequestedAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
    void                                                acsRequestedEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);
    void                                                acsDesiredAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
    void                                                acsDesiredEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);
    void                                                acsActualAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
    void                                                acsActualEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);

    void                                                skyRequestedAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
    void                                                skyRequestedEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);
    void                                                skyDesiredAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
    void                                                skyDesiredEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);
    void                                                skyActualAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus);
    void                                                skyActualEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus);

    void                                                pointingModel_callback(uint8_t ui8ParameterNumber, double dParameterValue);

    void                                                antennaStatus_callback(int64_t i64Timestamp_us, const std::string &strAntennaStatus, const std::string &strStatus);

    void                                                appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);
    void                                                antennaName_callback(const std::string &strAntennaName);
    void                                                antennaDiameter_callback(const std::string &strAntennaDiameter);
    void                                                antennaBeamwidth_callback(const std::string &strAntennaBeamwidth);
    void                                                antennaLongitude_callback(const std::string &strAntennaLongitude);
    void                                                antennaLatitude_callback(const std::string &strAntennaLatitude);

    //Noise diode values
    void                                                rNoiseDiodeInputSource_callback(int64_t i64Timestamp_us, const std::string &strInputSource, const std::string &strStatus);
    void                                                rNoiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeEnabled, const std::string &strStatus);
    void                                                rNoiseDiodeSelect_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSelect, const std::string &strStatus);
    void                                                rNoiseDiodePWMMark_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodePWMMark, const std::string &strStatus);
    void                                                rNoiseDiodePWMFrequency_callback(int64_t i64Timestamp_us, double dNoiseDiodePWMFrequency, const std::string &strStatus);


    //Global experiment values
    void                                                sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, const string &strStatus);


    //RF values
    void                                                bandSelectLcp_callback(int64_t i64Timestamp_us, bool bBandSelectLcp, const std::string &strStatus);
    void                                                bandSelectRcp_callback(int64_t i64Timestamp_us, bool bBandSelectRcp, const std::string &strStatus);
    void                                                frequencySky5GHz_callback(int64_t i64Timestamp_us, double dFrequencySky5GHz, const std::string &strStatus);
    void                                                frequencySky6_7GHz_callback(int64_t i64Timestamp_us, double dFrequencySky6_7GHz, const std::string &strStatus);
    void                                                receiverBandwidthLcp_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_Hz, const std::string &strStatus);
    void                                                receiverBandwidthRcp_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_Hz, const std::string &strStatus);
    void                                                receiverLcpAttenuation_callback(int64_t i64Timestamp_us, double dReceiverLcpAttenuation_dB, const std::string &strStatus);
    void                                                receiverRcpAttenuation_callback(int64_t i64Timestamp_us, double dReceiverRcpAttenuation_dB, const std::string &strStatus);

    //Env values
    void                                                envWindSpeed_callback(int64_t i64Timestamp_us, double dWindSpeed_mps, const std::string &strStatus);
    void                                                envWindDirection_callback(int64_t i64Timestamp_us, double dWindDirection_degrees, const std::string &strStatus);
    void                                                envTemperature_callback(int64_t i64Timestamp_us, double dTemperature_degreesC, const std::string &strStatus);
    void                                                envAbsolutePressure_callback(int64_t i64Timestamp_us, double dPressure_mbar, const std::string &strStatus);
    void                                                envRelativeHumidity_callback(int64_t i64Timestamp_us, double dHumidity_percent, const std::string &strStatus);

    //Roach register values
    //The data mode is spliced into the Roach 10 GbE data stream and interpreted from there so it is not necessary to implement these functions:
    //void                                                stokesEnabled_callback(int64_t i64Timestamp_us, bool bEnabled){}
    /////////////////////////////
    void                                                accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames);
    void                                                coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
    void                                                frequencyFs_callback(double dFrequencyFs_Hz);
    void                                                sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp);
    void                                                sizeOfFineFFT_callback(uint32_t u32FineFFTSize_nSamp);
    void                                                coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
    void                                                dspGain_callback(int64_t i64Timestamp_us, double dDspGain);
    void                                                attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB);
    void                                                attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB);
    //The noise diode information is spliced into the Roach 10 GbE data stream and interpreted from there so it is not necessary to implement these functions:
    void                                                noiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoideDiodeEnabled){}
    void                                                noiseDiodeDutyCycleEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeDutyCyleEnabled){}
    void                                                noiseDiodeDutyCycleOnDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums){}
    void                                                noiseDiodeDutyCycleOffDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums){}
    ////////////////////////////
    void                                                overflowsRegs_callback(int64_t i64Timestamp_us, uint32_t u32OverflowRegs);
    void                                                eth10GbEUp_callback(int64_t i64Timestamp_us, bool bEth10GbEUp);
    void                                                ppsCount_callback(int64_t i64Timestamp_us, uint32_t u32PPSCount);
    void                                                clockFrequency_callback(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz);

    //Other public functions
    std::string                                         makeFilename(const std::string &strDirectory, const std::string &strSuffix, int64_t i64Timestamp_us);

    void                                                waitForFileClosed();

    void                                                registerCallbackHandler(cCallbackInterface *pNewHandler);
    void                                                registerCallbackHandler(boost::shared_ptr<cCallbackInterface> pNewHandler);
    void                                                deregisterCallbackHandler(cCallbackInterface *pHandler);
    void                                                deregisterCallbackHandler(boost::shared_ptr<cCallbackInterface> pHandler);

    enum state
    {
        IDLE = 0,
        REQUEST_START,
        RECORDING,
        REQUEST_STOP
    };

private:
    int64_t                                             m_i64LastTimestamp_us;
    int64_t                                             m_i64FrameInterval_us;
    uint32_t                                            m_u32FrameSize_nVal;
    AVN::Spectrometer::digitiserType                    m_eLastDigitiserType;
    bool                                                m_bStreamChanged;

    cSpectrometerDataStreamInterpreter                  m_oDataStreamInterpreter;

    boost::shared_ptr<cSpectrometerHDF5OutputFile>      m_pHDF5File;

    boost::shared_mutex                                 m_oMutex;

    //Recording parameters
    std::string                                         m_strRecordingDirectory;
    uint32_t                                            m_u32FileSizeLimit_MB;
    std::string                                         m_strFilenameSuffix;
    std::string                                         m_strFilename;

    int64_t                                             m_i64RequestedStartTime_us;
    int64_t                                             m_i64ActualStartTime_us;
    int64_t                                             m_i64Duration_us;
    int64_t                                             m_i64StopTime_us;

    int64_t                                             m_i64RecordedDuration_us;

    int64_t                                             m_i64LastPrintTime_us;

    // Initial value struct
    cInitialValueSet                                    m_oInitialValueSet;

    //State machine
    void                                                setState(state eState);
    state                                               getState();
    state                                               m_eState;

    //Private thread safe accessors
    bool                                                rejectData();

    void                                                setRecordedDuration_us(int64_t i64Duration_us);

    //Callback handlers
    std::vector<cCallbackInterface*>                    m_vpCallbackHandlers;
    std::vector<boost::shared_ptr<cCallbackInterface> > m_vpCallbackHandlers_shared;
    boost::shared_mutex                                 m_oCallbackHandlersMutex;

    void                                                notifyAllRecordingStarted();
    void                                                notifyAllRecordingStopped();
};

#endif //
