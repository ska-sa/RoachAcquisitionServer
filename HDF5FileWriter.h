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

public:
    class cCallbackInterface
    {
    public:
        virtual void                                    recordingStarted_callback() = 0;
        virtual void                                    recordingStopped_callback() = 0;
    };

    cHDF5FileWriter(const std::string &strRecordingDirectory, uint32_t u32FileSizeLimit_MB);
    ~cHDF5FileWriter();

    void                                                startRecording(const std::string &strFilenamePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
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
    void                                                connected_callback(bool bConnected);

    //File recording control
    void                                                startRecording_callback(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                                stopRecording_callback();

    //Antenna values
    void                                                requestedAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg);
    void                                                requestedAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg);
    void                                                actualAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg);
    void                                                actualAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg);
    void                                                actualSourceOffsetAz_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg);
    void                                                actualSourceOffsetEl_callback(int64_t i64Timestamp_us, double dElevationOffset_deg);
    void                                                actualAntennaRA_callback(int64_t i64Timestamp_us, double dRighAscension_deg);
    void                                                actualAntennaDec_callback(int64_t i64Timestamp_us, double dDeclination_deg);

    void                                                antennaStatus_callback(int64_t i64Timestamp_us, const std::string &strStatus);
    void                                                motorTorqueAzMaster_callback(int64_t i64Timestamp_us, double dAzMaster_mNm);
    void                                                motorTorqueAzSlave_callback(int64_t i64Timestamp_us, double dAzSlave_mNm);
    void                                                motorTorqueElMaster_callback(int64_t i64Timestamp_us, double dElMaster_mNm);
    void                                                motorTorqueElSlave_callback(int64_t i64Timestamp_us, double dElSlave_mNm);
    void                                                appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);

    //Noise diode values
    void                                                noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState);
    void                                                noiseDiodeSource_callback(int64_t i64Timestamp_us, const std::string &strNoiseSource);
    void                                                noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A);

    //Global experiment values
    void                                                sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);


    //RF values
    void                                                frequencyRFChan0_callback(int64_t i64Timestamp_us, double dFreqencyRFChan0_MHz);
    void                                                frequencyRFChan1_callback(int64_t i64Timestamp_us, double dFreqencyRFChan1_MHz);
    void                                                frequencyLO0Chan0_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz);
    void                                                frequencyLO0Chan1_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz);
    void                                                frequencyLO1_callback(int64_t i64Timestamp_us, double dFrequencyLO1_MHz);
    void                                                receiverBandwidthChan0_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz);
    void                                                receiverBandwidthChan1_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz);

    void                                                accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames);
    void                                                coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
    void                                                frequencyFs_callback(double dFrequencyFs_MHz);
    void                                                sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp);
    void                                                sizeOfFineFFT_callback(uint32_t u32FineFFTSize_nSamp);
    void                                                coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
    void                                                attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB);
    void                                                attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB);

    //Other public functions
    std::string                                         makeFilename(const std::string &strDirectory, const std::string &strPrefix, int64_t i64Timestamp_us);

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
    std::string                                         m_strFilenamePrefix;
    std::string                                         m_strFilename;

    int64_t                                             m_i64RequestedStartTime_us;
    int64_t                                             m_i64ActualStartTime_us;
    int64_t                                             m_i64Duration_us;
    int64_t                                             m_i64StopTime_us;

    int64_t                                             m_i64RecordedDuration_us;

    int64_t                                             m_i64LastPrintTime_us;

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
