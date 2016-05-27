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
        virtual void                                    requestedAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg) = 0;
        virtual void                                    requestedAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg) = 0;
        virtual void                                    actualAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg) = 0;
        virtual void                                    actualAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg) = 0;
        virtual void                                    actualSourceOffsetAz_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg) = 0;
        virtual void                                    actualSourceOffsetEl_callback(int64_t i64Timestamp_us, double dElevationOffset_deg) = 0;
        virtual void                                    actualAntennaRA_callback(int64_t i64Timestamp_us, double dRighAscension_deg) = 0;
        virtual void                                    actualAntennaDec_callback(int64_t i64Timestamp_us, double dDeclination_deg) = 0;

        virtual void                                    antennaStatus_callback(int64_t i64Timestamp_us, const std::string &strStatus) = 0;
        virtual void                                    motorTorqueAzMaster_callback(int64_t i64Timestamp_us, double dAzMaster_mNm) = 0;
        virtual void                                    motorTorqueAzSlave_callback(int64_t i64Timestamp_us, double dAzSlave_mNm) = 0;
        virtual void                                    motorTorqueElMaster_callback(int64_t i64Timestamp_us, double dElMaster_mNm) = 0;
        virtual void                                    motorTorqueElSlave_callback(int64_t i64Timestamp_us, double dElSlave_mNm) = 0;
        virtual void                                    appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams) = 0;

        //Noise diode values
        virtual void                                    noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState) = 0;
        virtual void                                    noiseDiodeSource_callback(int64_t i64Timestamp_us, const std::string &strNoiseSource) = 0;
        virtual void                                    noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A) = 0;

        //Global experiment values
        virtual void                                    sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg) = 0;


        //RF values
        virtual void                                    frequencyRFChan0_callback(int64_t i64Timestamp_us, double dFreqencyRFChan0_MHz) = 0;
        virtual void                                    frequencyRFChan1_callback(int64_t i64Timestamp_us, double dFreqencyRFChan1_MHz) = 0;
        virtual void                                    frequencyLO0Chan0_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz) = 0;
        virtual void                                    frequencyLO0Chan1_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz) = 0;
        virtual void                                    frequencyLO1_callback(int64_t i64Timestamp_us, double dFrequencyLO1_MHz) = 0;
        virtual void                                    receiverBandwidthChan0_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz) = 0;
        virtual void                                    receiverBandwidthChan1_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz) = 0;
    };

    cStationControllerKATCPClient(const std::string &strServerAddress, uint16_t u16Port = 7147);
    cStationControllerKATCPClient();
    ~cStationControllerKATCPClient();

private:
    //Implement from base class
    void                                                onConnected();
    void                                                processKATCPMessage(const std::vector<std::string> &vstrMessageTokens);

    //Print KATCP Debug information
    void                                                printVersion(const std::string &strVersion);
    void                                                printBuildState(const std::string &strBuilState);

    //Notifications sent to all callback handlers:

    //Recording control:
    void                                                sendStartRecording(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                                sendStopRecording();

    //Antenna values
    void                                                sendRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg);
    void                                                sendRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg);
    void                                                sendActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg);
    void                                                sendActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg);
    void                                                sendActualSourceOffsetAz(int64_t i64Timestamp_us, double dAzimuthOffset_deg);
    void                                                sendActualSourceOffsetEl(int64_t i64Timestamp_us, double dElevationOffset_deg);
    void                                                sendActualAntennaRA(int64_t i64Timestamp_us, double dRighAscension_deg);
    void                                                sendActualAntennaDec(int64_t i64Timestamp_us, double dDeclination_deg);

    void                                                sendAntennaStatus(int64_t i64Timestamp_us, std::string strAntennaStatus);
    void                                                sendMotorTorqueAzMaster(int64_t i64Timestamp_us, double dAzMaster_nNm);
    void                                                sendMotorTorqueAzSlave(int64_t i64Timestamp_us, double dAzSlave_nNm);
    void                                                sendMotorTorqueElMaster(int64_t i64Timestamp_us, double dElMaster_nNm);
    void                                                sendMotorTorqueElSlave(int64_t i64Timestamp_us, double dElSlave_nNm);
    void                                                sendAppliedPointingModel(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);

    //Noise diode values
    void                                                sendNoiseDiodeSoftwareState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState);
    void                                                sendNoiseDiodeSource(int64_t i64Timestamp_us, const std::string &strNoiseSource);
    void                                                sendNoiseDiodeCurrent(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A);

    //Global experiment values
    void                                                sendSourceSelection(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);

    //RFE values
    void                                                sendFrequencyRFChan0(int64_t i64Timestamp_us, double dFreqencyRF_MHz);
    void                                                sendFrequencyRFChan1(int64_t i64Timestamp_us, double dFreqencyRF_MHz);
    void                                                sendFrequencyLO0Chan0(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz);
    void                                                sendFrequencyLO0Chan1(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz);
    void                                                sendFrequencyLO1(int64_t i64Timestamp_us, double dFrequencyLO1_MHz);
    void                                                sendReceiverBandwidthChan0(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz);
    void                                                sendReceiverBandwidthChan1(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz);
};

#endif // STATION_CONTROLLER_KATCP_CLIENT_H
