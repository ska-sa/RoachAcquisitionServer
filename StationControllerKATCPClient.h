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
        virtual void                                    requestedAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus) = 0;
        virtual void                                    requestedAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    actualAntennaAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus) = 0;
        virtual void                                    actualAntennaEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    actualSourceOffsetAz_callback(int64_t i64Timestamp_us, double dAzimuthOffset_deg, const std::string &strStatus) = 0;
        virtual void                                    actualSourceOffsetEl_callback(int64_t i64Timestamp_us, double dElevationOffset_deg, const std::string &strStatus) = 0;
        virtual void                                    actualAntennaRA_callback(int64_t i64Timestamp_us, double dRighAscension_deg, const std::string &strStatus) = 0;
        virtual void                                    actualAntennaDec_callback(int64_t i64Timestamp_us, double dDeclination_deg, const std::string &strStatus) = 0;

        virtual void                                    antennaStatus_callback(int64_t i64Timestamp_us, const std::string &strAntennaStatus, const std::string &strStatus) = 0;
        virtual void                                    motorTorqueAzMaster_callback(int64_t i64Timestamp_us, double dAzMaster_mNm, const std::string &strStatus) = 0;
        virtual void                                    motorTorqueAzSlave_callback(int64_t i64Timestamp_us, double dAzSlave_mNm, const std::string &strStatus) = 0;
        virtual void                                    motorTorqueElMaster_callback(int64_t i64Timestamp_us, double dElMaster_mNm, const std::string &strStatus) = 0;
        virtual void                                    motorTorqueElSlave_callback(int64_t i64Timestamp_us, double dElSlave_mNm, const std::string &strStatus) = 0;

        //Noise diode values
        virtual void                                    noiseDiodeSoftwareState_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const std::string &strStatus) = 0;
        virtual void                                    noiseDiodeSource_callback(int64_t i64Timestamp_us, const std::string &strNoiseSource, const std::string &strStatus) = 0;
        virtual void                                    noiseDiodeCurrent_callback(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A, const std::string &strStatus) = 0;

        //Global experiment values
        virtual void                                    sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg) = 0;


        //RF values
        virtual void                                    frequencySelectChan0_callback(int64_t i64Timestamp_us, bool bFrequencySelectChan0, const std::string &strStatus) = 0;
        virtual void                                    frequencySelectChan1_callback(int64_t i64Timestamp_us, bool bFrequencySelectChan1, const std::string &strStatus) = 0;
        virtual void                                    frequencyLO0Chan0_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_Hz, const std::string &strStatus) = 0;
        virtual void                                    frequencyLO0Chan1_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_Hz, const std::string &strStatus) = 0;
        virtual void                                    frequencyLO1_callback(int64_t i64Timestamp_us, double dFrequencyLO1_Hz, const std::string &strStatus) = 0;
        virtual void                                    receiverBandwidthChan0_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_Hz, const std::string &strStatus) = 0;
        virtual void                                    receiverBandwidthChan1_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_Hz, const std::string &strStatus) = 0;

        /* TODO: remove antenna configuration functions.
        virtual void                                    antennaName_callback(const std::string &strAntennaName) = 0;
        virtual void                                    antennaDiameter_callback(const std::string &strAntennaDiameter) = 0;
        virtual void                                    antennaBeamwidth_callback(const std::string &strAntennaBeamwidth) = 0;
        virtual void                                    antennaLongitude_callback(const std::string &strAntennaLongitude) = 0;
        virtual void                                    antennaLatitude_callback(const std::string &strAntennaLatitude) = 0;
        virtual void                                    appliedPointingModel_callback(const std::string &strModelName, const std::vector<double> &vdPointingModelParams) = 0;
        */
    };

    cStationControllerKATCPClient(const std::string &strServerAddress, uint16_t u16Port = 7147);
    cStationControllerKATCPClient();
    ~cStationControllerKATCPClient();

    void subscribeSensorData();
    void unsubscribeSensorData();

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
    void                                                sendRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);
    void                                                sendActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);
    void                                                sendActualSourceOffsetAz(int64_t i64Timestamp_us, double dAzimuthOffset_deg, const std::string &strStatus);
    void                                                sendActualSourceOffsetEl(int64_t i64Timestamp_us, double dElevationOffset_deg, const std::string &strStatus);
    void                                                sendActualAntennaRA(int64_t i64Timestamp_us, double dRighAscension_deg, const std::string &strStatus);
    void                                                sendActualAntennaDec(int64_t i64Timestamp_us, double dDeclination_deg, const std::string &strStatus);

    void                                                sendAntennaStatus(int64_t i64Timestamp_us, std::string strAntennaStatus, const std::string &strStatus);
    void                                                sendMotorTorqueAzMaster(int64_t i64Timestamp_us, double dAzMaster_nNm, const std::string &strStatus);
    void                                                sendMotorTorqueAzSlave(int64_t i64Timestamp_us, double dAzSlave_nNm, const std::string &strStatus);
    void                                                sendMotorTorqueElMaster(int64_t i64Timestamp_us, double dElMaster_nNm, const std::string &strStatus);
    void                                                sendMotorTorqueElSlave(int64_t i64Timestamp_us, double dElSlave_nNm, const std::string &strStatus);

    //Noise diode values
    void                                                sendNoiseDiodeSoftwareState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const std::string &strStatus);
    void                                                sendNoiseDiodeSource(int64_t i64Timestamp_us, const std::string &strNoiseSource, const std::string &strStatus);
    void                                                sendNoiseDiodeCurrent(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A, const std::string &strStatus);

    //Global experiment values
    void                                                sendSourceSelection(int64_t i64Timestamp_us, const std::string &strSourceName, double dRighAscension_deg, double dDeclination_deg);

    //RFE values
    void                                                sendFrequencySelectChan0(int64_t i64Timestamp_us, double dFreqencyRF_MHz, const std::string &strStatus);
    void                                                sendFrequencySelectChan1(int64_t i64Timestamp_us, double dFreqencyRF_MHz, const std::string &strStatus);
    void                                                sendFrequencyLO0Chan0(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz, const std::string &strStatus);
    void                                                sendFrequencyLO0Chan1(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz, const std::string &strStatus);
    void                                                sendFrequencyLO1(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, const std::string &strStatus);
    void                                                sendReceiverBandwidthChan0(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz, const std::string &strStatus);
    void                                                sendReceiverBandwidthChan1(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz, const std::string &strStatus);
    void                                                sendLcpAttenuation(int64_t i64Timestamp_us, double dLcpAttenuation_dB, const std::string &strStatus);
    void                                                sendRcpAttenuation(int64_t i64Timestamp_us, double dRcpAttenuation_dB, const std::string &strStatus);

    /* TODO: Antenna configuration info marked for removal.
    //Antenna info values
    void                                                sendAntennaName(const std::string &strAntennaName);
    void                                                sendAntennaBeamwidth(const std::string &strAntennaBeamwidth);
    void                                                sendAntennaDelayModel(const std::string &strAntennaDelayModel);
    void                                                sendAntennaDiameter(const std::string &strAntennaDiameter);
    void                                                sendAntennaLatitude(const std::string &strAntennaLatitude);
    void                                                sendAntennaLongitude(const std::string &strAntennaLongitude);
    void                                                sendAppliedPointingModel(const std::string &strModelName, const std::vector<double> &vdPointingModelParams);


    //Member storage vars:
    std::string                                         m_strAntennaStatus;
    std::string                                         m_strAppliedPointingModel;
    std::string                                         m_strPointingModelName;
    std::string                                         m_strSourceSelection;
    std::string                                         m_strAntennaDelayModel;
    std::string                                         m_strAntennaName;
    */
    std::vector<std::string>                            m_vstrSensorNames;
};

#endif // STATION_CONTROLLER_KATCP_CLIENT_H
