#ifndef STATION_CONTROLLER_KATCP_CLIENT_H
#define STATION_CONTROLLER_KATCP_CLIENT_H

//System includes

//Library includes

//Local includes
#include "AVNAppLibs/KATCP/KATCPClientBase.h"

//Implementation of KATCP client for getting values from the station controller

class cStationControllerKATCPClient : public cKATCPClientBase
{
public:
    class cCallbackInterface : public cKATCPClientBase::cCallbackInterface
    {
    public:       
        //File recording control
        virtual void                                    startRecording_callback(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us) = 0;
        virtual void                                    stopRecording_callback() = 0;
        
        //Antenna values
        virtual void                                    requestedAntennaAzEl_callback(int64_t i64Timestamp_us,
                                                                             double dAzimuth_deg, double dElevation_deg) = 0;
        virtual void                                    actualAntennaAzEl_callback(int64_t i64Timestamp_us,
                                                                          double dAzimuth_deg, double dElevation_deg) = 0;
        virtual void                                    actualSourceOffsetAzEl_callback(int64_t i64Timestamp_us,
                                                                               double dAzimuthOffset_deg, double dElevationOffset_deg) = 0;
        virtual void                                    actualAntennaRADec_callback(int64_t i64Timestamp_us,
                                                                           double dRighAscension_deg, double dDeclination_deg) = 0;
        
        virtual void                                    antennaStatus_callback(int64_t i64Timestamp_us, int32_t i32AntennaStatus, const std::string &strAntennaStatus) = 0;
        virtual void                                    motorTorques_callback(int64_t i64Timestamp_us, double dAz0_Nm, double dAz1_Nm, double dEl0, double dEl1) = 0;
        virtual void                                    appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams) = 0;
        
        //Noise diode values
        virtual void                                    noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState) = 0;
        virtual void                                    noiseDiodeSource_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSource, const std::string &strNoiseSource) = 0;
        virtual void                                    noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A) = 0;
        
        //Global experiment values
        virtual void                                    sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg) = 0;
        
        //RFE values
        virtual void                                    frequencyRF_callback(int64_t i64Timestamp_us, double dFreqencyRF_MHz) = 0;
        virtual void                                    frequencyLOs_callback(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, double dFrequencyLO2_MHz) = 0;
        virtual void                                    bandwidthIF_callback(int64_t i64Timestamp_us, double dBandwidthIF_MHz) = 0;
    };
    
    cStationControllerKATCPClient(const std::string &strServerAddress, uint16_t u16Port = 7147);
    cStationControllerKATCPClient();
    ~cStationControllerKATCPClient();
    
private:
    //Implement from base class
    void                                                processKATCPMessage(const std::vector<std::string> &vstrMessageTokens);

    //Notifications sent to all callback handlers:

    //Recording control:
    void                                                sendStartRecording(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                                sendStopRecording();

    //Antenna values
    void                                                sendRequestedAntennaAzEl(int64_t i64Timestamp_us,
                                                                         double dAzimuth_deg, double dElevation_deg);
    void                                                sendActualAntennaAzEl(int64_t i64Timestamp_us,
                                                                      double dAzimuth_deg, double dElevation_deg);
    void                                                sendActualSourceOffsetAzEl(int64_t i64Timestamp_us,
                                                                           double dAzimuthOffset_deg, double dElevationOffset_deg);
    void                                                sendActualAntennaRADec(int64_t i64Timestamp_us,
                                                                       double dRighAscension_deg, double dDeclination_deg);
    
    void                                                sendAntennaStatus(int64_t i64Timestamp_us, int32_t i32AntennaStatus, std::string strAntennaStatus);
    void                                                sendMotorTorques(int64_t i64Timestamp_us, double dAz0_Nm, double dAz1_Nm, double dEl0_Nm, double dEl1_Nm);
    void                                                sendAppliedPointingModel(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);
    
    //Noise diode values
    void                                                sendNoiseDiodeSoftwareState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState);
    void                                                sendNoiseDiodeSource(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSource, const std::string &strNoiseSource);
    void                                                sendNoiseDiodeCurrent(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A);
    
    //Global experiment values
    void                                                sendSourceSelection(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);
    
    //RFE values
    void                                                sendFrequencyRF(int64_t i64Timestamp_us, double dFreqencyRF_MHz);
    void                                                sendFrequencyLOs(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, double dFrequencyLO2_MHz);
    void                                                sendBandwidthIF(int64_t i64Timestamp_us, double dBandwidthIF_MHz);
};

#endif // STATION_CONTROLLER_KATCP_CLIENT_H
