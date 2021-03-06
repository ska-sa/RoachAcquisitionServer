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
        virtual void                                    startRecording_callback(const std::string &strFileSuffix, int64_t i64StartTime_us, int64_t i64Duration_us) = 0;
        virtual void                                    stopRecording_callback() = 0;

        //Antenna values
        virtual void                                    acsRequestedAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus) = 0;
        virtual void                                    acsRequestedEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    acsDesiredAz_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    acsDesiredEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    acsActualAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus) = 0;
        virtual void                                    acsActualEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;

        virtual void                                    skyRequestedAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus) = 0;
        virtual void                                    skyRequestedEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    skyDesiredAz_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    skyDesiredEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;
        virtual void                                    skyActualAz_callback(int64_t i64Timestamp_us, double dAzimuth_deg, const std::string &strStatus) = 0;
        virtual void                                    skyActualEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const std::string &strStatus) = 0;

        virtual void                                    pointingModel_callback(uint8_t i8ParameterNumber, double dParameterValue) = 0;

        virtual void                                    antennaStatus_callback(int64_t i64Timestamp_us, const std::string &strAntennaStatus, const std::string &strStatus) = 0;

        //Global experiment values
        virtual void                                    sourceSelection_callback(int64_t i64Timestamp_us, const std::string &strSourceName, const std::string &strStatus) = 0;

        //RF values
        virtual void                                    frequencySelectLcp_callback(int64_t i64Timestamp_us, bool bFrequencySelectChan0, const std::string &strStatus) = 0;
        virtual void                                    frequencySelectRcp_callback(int64_t i64Timestamp_us, bool bFrequencySelectChan1, const std::string &strStatus) = 0;
        virtual void                                    frequencyLOIntermediate5GHz_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_Hz, const std::string &strStatus) = 0;
        virtual void                                    frequencyLOIntermediate6_7GHz_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_Hz, const std::string &strStatus) = 0;
        virtual void                                    frequencyLOFinal_callback(int64_t i64Timestamp_us, double dFrequencyLO1_Hz, const std::string &strStatus) = 0;
        virtual void                                    receiverBandwidthLcp_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_Hz, const std::string &strStatus) = 0;
        virtual void                                    receiverBandwidthRcp_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_Hz, const std::string &strStatus) = 0;
        virtual void                                    receiverLcpAttenuation_callback(int64_t i64Timestamp_us, double dLCPAttenuation_dB, const std::string &strStatus) = 0;
        virtual void                                    receiverRcpAttenuation_callback(int64_t i64Timestamp_us, double dRCPAttenuation_dB, const std::string &strStatus) = 0;

        //Noise diode values
        virtual void                                    rNoiseDiodeInputSource_callback(int64_t i64Timestamp_us, const std::string &strNoiseDiodeInputSource, const std::string &strStatus) = 0;
        virtual void                                    rNoiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeEnabled, const std::string &strStatus) = 0;
        virtual void                                    rNoiseDiodeSelect_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSelect, const std::string &strStatus) = 0;
        virtual void                                    rNoiseDiodePWMMark_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodePWMMark, const std::string &strStatus) = 0;
        virtual void                                    rNoiseDiodePWMFrequency_callback(int64_t i64Timestamp_us, double dNoiseDiodePWMFrequency, const std::string &strStatus) = 0;

        //Env values
        virtual void                                    envWindSpeed_callback(int64_t i64Timestamp_us, double dWindSpeed_mps, const std::string &strStatus) = 0;
        virtual void                                    envWindDirection_callback(int64_t i64Timestamp_us, double dWindDirection_degrees, const std::string &strStatus) = 0;
        virtual void                                    envTemperature_callback(int64_t i64Timestamp_us, double dTemperature_degreesC, const std::string &strStatus) = 0;
        virtual void                                    envAbsolutePressure_callback(int64_t i64Timestamp_us, double dPressure_mbar, const std::string &strStatus) = 0;
        virtual void                                    envRelativeHumidity_callback(int64_t i64Timestamp_us, double dHumidity_percent, const std::string &strStatus) = 0;
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
    void                                                sendStartRecording(const std::string &strFileSuffix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                                sendStopRecording();

    //Antenna values
    void                                                sendAcsRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendAcsRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);
    void                                                sendAcsDesiredAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendAcsDesiredAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);
    void                                                sendAcsActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendAcsActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);

    void                                                sendSkyRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendSkyRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);
    void                                                sendSkyDesiredAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendSkyDesiredAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);
    void                                                sendSkyActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const std::string &strStatus);
    void                                                sendSkyActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const std::string &strStatus);

    void                                                sendPointingModelParameter(uint8_t ui8ParameterNumber, double dParameterValue);
    // TODO: Figure out what to do about this one.
    void                                                sendAntennaStatus(int64_t i64Timestamp_us, std::string strAntennaStatus, const std::string &strStatus);

    //Noise diode values
    void                                                sendNoiseDiodeState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const std::string &strStatus);

    //Global experiment values
    void                                                sendSourceSelection(int64_t i64Timestamp_us, const std::string &strSourceName, const std::string &strStatus);

    //RFE values
    void                                                sendFrequencySelectLcp(int64_t i64Timestamp_us, bool bFreqencyRFChan0_MHz, const std::string &strStatus);
    void                                                sendFrequencySelectRcp(int64_t i64Timestamp_us, bool bFreqencyRF_MHz, const std::string &strStatus);
    void                                                sendFrequencyLOIntermediate5GHz(int64_t i64Timestamp_us, double dFrequencyIntermediate5GHz_MHz, const std::string &strStatus);
    void                                                sendFrequencyLOIntermediate6_7GHz(int64_t i64Timestamp_us, double dFrequencyLOIntermediate6_7GHz_MHz, const std::string &strStatus);
    void                                                sendFrequencyLOFinal(int64_t i64Timestamp_us, double dFrequencyLOFinal_MHz, const std::string &strStatus);
    void                                                sendReceiverBandwidthLcp(int64_t i64Timestamp_us, double dReceiverBandwidthLcp_MHz, const std::string &strStatus);
    void                                                sendReceiverBandwidthRcp(int64_t i64Timestamp_us, double dReceiverBandwidthRcp_MHz, const std::string &strStatus);
    void                                                sendReceiverLcpAttenuation(int64_t i64Timestamp_us, double dLcpAttenuation_dB, const std::string &strStatus);
    void                                                sendReceiverRcpAttenuation(int64_t i64Timestamp_us, double dRcpAttenuation_dB, const std::string &strStatus);

    // Environment values
    void                                                sendWindSpeed(int64_t i64Timestamp_us, double dWindSpeed_mps, const std::string &strStatus);
    void                                                sendWindDirection(int64_t i64Timestamp_us, double dWindDirection_degrees, const std::string &strStatus);
    void                                                sendTemperature(int64_t i64Timestamp_us, double dTemperature_degreesC, const std::string &strStatus);
    void                                                sendAbsolutePressure(int64_t i64Timestamp_us, double dPressure_mbar, const std::string &strStatus);
    void                                                sendRelativeHumidity(int64_t i64Timestamp_us, double dHumidity_percent, const std::string &strStatus);

    // Sensor-sampling commands on subscribing.
    std::vector<std::string>                            m_vstrSensorSampling;
};

#endif // STATION_CONTROLLER_KATCP_CLIENT_H
