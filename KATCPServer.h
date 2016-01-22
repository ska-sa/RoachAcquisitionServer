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

class cKATCPServer
{
public:
    //cKATCPServer is pure static and therefore cannot derive the non-static callback interfaces

    //So define a new non-static derived classes here as callback handlers and create instances as
    //static members of the cKATCPServer class.

    class cHDF5FileWriterCallbackHandler : public cHDF5FileWriter::cCallbackInterface
    {
        //Callback functions called from HDF5FileWriter

        void recordingStarted_callback();
        void recordingStopped_callback();
    };

    class cKATCPClientCallbackHandler : public cRoachKATCPClient::cCallbackInterface, public cStationControllerKATCPClient
    {
        //Callback functions called from the KATCPClient

        void                                                connected_callback(bool bConnected);

        //File recording control
        void                                                startRecording_callback(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
        void                                                stopRecording_callback();

        //Antenna values
        void                                                requestedAntennaAzEl_callback(int64_t i64Timestamp_us,
                                                                             double dAzimuth_deg, double dElevation_deg);
        void                                                actualAntennaAzEl_callback(int64_t i64Timestamp_us,
                                                                          double dAzimuth_deg, double dElevation_deg);
        void                                                actualSourceOffsetAzEl_callback(int64_t i64Timestamp_us,
                                                                               double dAzimuthOffset_deg, double dElevationOffset_deg);
        void                                                actualAntennaRADec_callback(int64_t i64Timestamp_us,
                                                                           double dRighAscension_deg, double dDeclination_deg);

        void                                                antennaStatus_callback(int64_t i64Timestamp_us, int32_t i32AntennaStatus, const std::string &strStatus);
        void                                                motorTorques_callback(int64_t i64Timestamp_us, double dAz0_Nm, double dAz1_Nm, double dEl0_Nm, double dEl1_Nm);
        void                                                appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);

        //Noise diode values
        void                                                noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState);
        void                                                noiseDiodeSource_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSource, const std::string &strNoiseSource);
        void                                                noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A);

        //Global experiment values
        void                                                sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);


        //RF values
        void                                                frequencyRF_callback(int64_t i64Timestamp_us, double dFreqencyRF_MHz);
        void                                                frequencyLOs_callback(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, double dFrequencyLO2_MHz);
        void                                                bandwidthIF_callback(int64_t i64Timestamp_us, double dBandwidthIF_MHz);

        //Roach values
        void                                                accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames);
        void                                                narrowBandChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
        void                                                frequencyFs_callback(double dFrequencyFs_MHz);
        void                                                sizeOfFFTs_callback(uint32_t u32CoarseSize_nSamp, uint32_t u32FineSize_nSamp);
        void                                                coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
        void                                                adcAttenuation_callback(int64_t i64Timestamp_us, double dAttenuationChan0_dB, double dAttenuationChan1_dB);
        
    };

    cKATCPServer(const std::string &strListenInterface = std::string("0.0.0.0"), uint16_t u16Port = 7147, uint32_t u32MaxClients = 5);
    cKATCPServer();
    ~cKATCPServer();

    static void                                             setFileWriter(boost::shared_ptr<cHDF5FileWriter> pFileWriter);
    static void                                             setRoachKATCPClient(boost::shared_ptr<cRoachKATCPClient> pRoachKATCPClient);
    static void                                             setStationControllerKATCPClient(boost::shared_ptr<cStationControllerKATCPClient> pKATCPClient);

    static void                                             startServer(const std::string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients);
    static void                                             stopServer();

protected:
    //Callback handlers (Need to have new actual objects here as we cannot derive the callback handler classes with this class as it is static.)
    static cHDF5FileWriterCallbackHandler                   m_oHDF5FileWriterCallBackHandler;
    static cKATCPClientCallbackHandler                      m_oKATCPClientCallbackHandler;

    static void                                             serverThreadFunction();

    static struct katcp_dispatch                            *m_pKATCPDispatch;

    //KATCTP server callbacks
    static int32_t                                          startRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          stopRecording_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          getRecordingInfo_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                                          getRecordingStatus_KATCPCallback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);

    static boost::shared_ptr<cHDF5FileWriter>               m_pFileWriter;
    static boost::shared_ptr<cRoachKATCPClient>             m_pRoachKATCPClient;
    static boost::shared_ptr<cStationControllerKATCPClient> m_pStationControllerKATCPClient;

    //Threads
    static boost::scoped_ptr<boost::thread>                 m_pKATCPThread;

    //Members description operation state
    static std::string                                      m_strListenInterface;
    static uint16_t                                         m_u16Port;
    static uint32_t                                         m_u32MaxClients;

};

#endif // KATCTP_SERVER_H
