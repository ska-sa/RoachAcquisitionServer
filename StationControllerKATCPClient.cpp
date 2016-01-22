
//System includes

//Library includes

//Local includes
#include "StationControllerKATCPClient.h"

using namespace std;

cStationControllerKATCPClient::cStationControllerKATCPClient(const string &strServerAddress, uint16_t u16Port) :
    cKATCPClientBase(strServerAddress, u16Port)
{
}

cStationControllerKATCPClient::cStationControllerKATCPClient() :
    cKATCPClientBase()
{
}

cStationControllerKATCPClient::~cStationControllerKATCPClient()
{
}

void cStationControllerKATCPClient::processKATCPMessage(const vector<string> &vstrTokens)
{
    try
    {
        if( !vstrTokens[0].compare("#requestedAntennaAzEl") )
        {
            sendRequestedAntennaAzEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), strtod(vstrTokens[3].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }


    try
    {
        if(!vstrTokens[0].compare("#actualAntennaAzEl"))
        {
            sendActualAntennaAzEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), strtod(vstrTokens[3].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualSourceOffsetAzEl"))
        {
            sendActualSourceOffsetAzEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), strtod(vstrTokens[3].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualAntennaRADec"))
        {
            sendActualAntennaRADec( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), strtod(vstrTokens[3].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#antennaStatus"))
        {
            sendAntennaStatus( strtoll(vstrTokens[1].c_str(), NULL, 10), strtol(vstrTokens[2].c_str(), NULL, 10), vstrTokens[3] );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#motorTorques"))
        {
            double dAz0_Nm, dAz1_Nm, dEl0_Nm, dEl1_Nm;

            dAz0_Nm =  strtod(vstrTokens[2].c_str(), NULL);
            dAz1_Nm =  strtod(vstrTokens[3].c_str(), NULL);
            dEl0_Nm =  strtod(vstrTokens[4].c_str(), NULL);
            dEl1_Nm =  strtod(vstrTokens[5].c_str(), NULL);

            sendMotorTorques( strtoll(vstrTokens[1].c_str(), NULL, 10), dAz0_Nm, dAz1_Nm, dEl0_Nm, dEl1_Nm);
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#appliedPointingModel"))
        {
            string strPointingModelName = vstrTokens[1];

            vector<double> vdPointingModel;
            for(uint32_t ui = 2; ui < vstrTokens.size(); ui++)
            {
                vdPointingModel.push_back( strtod(vstrTokens[ui].c_str(), NULL) );
            }

            sendAppliedPointingModel( strPointingModelName, vdPointingModel );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#noiseDiodeSoftwareState"))
        {
            sendNoiseDiodeSoftwareState( strtoll(vstrTokens[1].c_str(), NULL, 10), strtol(vstrTokens[2].c_str(), NULL, 10) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#noiseDiodeSource"))
        {
            sendNoiseDiodeSource( strtoll(vstrTokens[1].c_str(), NULL, 10), strtol(vstrTokens[2].c_str(), NULL, 10), vstrTokens[3] );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#noideDiodeCurrent"))
        {
            sendNoiseDiodeCurrent( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#sourceSelection"))
        {
            sendSourceSelection( strtoll(vstrTokens[1].c_str(), NULL, 10), vstrTokens[2], strtod(vstrTokens[3].c_str(), NULL), strtod(vstrTokens[4].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyRF"))
        {
            sendFrequencyRF( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyLOs"))
        {
            sendFrequencyLOs( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#bandwidthIF"))
        {
            sendBandwidthIF( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }
}

void  cStationControllerKATCPClient::sendStartRecording(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->startRecording_callback(strFilePrefix, i64StartTime_us, i64Duration_us);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->startRecording_callback(strFilePrefix, i64StartTime_us, i64Duration_us);
    }
}

void  cStationControllerKATCPClient::sendStopRecording()
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->stopRecording_callback();
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->stopRecording_callback();
    }
}

void cStationControllerKATCPClient::sendRequestedAntennaAzEl(int64_t i64Timestamp_us, double dAzimuth_deg, double dElevation_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->requestedAntennaAzEl_callback(i64Timestamp_us, dAzimuth_deg, dElevation_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->requestedAntennaAzEl_callback(i64Timestamp_us, dAzimuth_deg, dElevation_deg);
    }
}

void cStationControllerKATCPClient::sendActualAntennaAzEl(int64_t i64Timestamp_us, double dAzimuth_deg, double dElevation_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaAzEl_callback(i64Timestamp_us, dAzimuth_deg, dElevation_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaAzEl_callback(i64Timestamp_us, dAzimuth_deg, dElevation_deg);
    }
}

void cStationControllerKATCPClient::sendActualSourceOffsetAzEl(int64_t i64Timestamp_us, double dAzimuthOffset_deg, double dElevationOffset_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualSourceOffsetAzEl_callback(i64Timestamp_us, dAzimuthOffset_deg, dElevationOffset_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualSourceOffsetAzEl_callback(i64Timestamp_us, dAzimuthOffset_deg, dElevationOffset_deg);
    }
}

void cStationControllerKATCPClient::sendActualAntennaRADec(int64_t i64Timestamp_us, double dRighAscension_deg, double dDeclination_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaRADec_callback(i64Timestamp_us, dRighAscension_deg, dDeclination_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaRADec_callback(i64Timestamp_us, dRighAscension_deg, dDeclination_deg);
    }
}

void cStationControllerKATCPClient::sendAntennaStatus(int64_t i64Timestamp_us, int32_t i32AntennaStatus, std::string strAntennaStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaStatus_callback(i64Timestamp_us, i32AntennaStatus, strAntennaStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaStatus_callback(i64Timestamp_us, i32AntennaStatus, strAntennaStatus);
    }
}

void cStationControllerKATCPClient::sendMotorTorques(int64_t i64Timestamp_us, double dAz0_Nm, double dAz1_Nm, double dEl0_Nm, double dEl1_Nm)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorques_callback(i64Timestamp_us, dAz0_Nm, dAz1_Nm, dEl0_Nm, dEl1_Nm);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorques_callback(i64Timestamp_us, dAz0_Nm, dAz1_Nm, dEl0_Nm, dEl1_Nm);
    }
}

void cStationControllerKATCPClient::sendAppliedPointingModel(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->appliedPointingModel_callback(strModelName, vdPointingModelParams);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->appliedPointingModel_callback(strModelName, vdPointingModelParams);
    }
}

void cStationControllerKATCPClient::sendNoiseDiodeSoftwareState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeSoftwareState_callback(i64Timestamp_us, i32NoiseDiodeState);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeSoftwareState_callback(i64Timestamp_us, i32NoiseDiodeState);
    }
}

void cStationControllerKATCPClient::sendNoiseDiodeSource(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSource, const string &strNoiseSource)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeSource_callback(i64Timestamp_us, i32NoiseDiodeSource, strNoiseSource);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeSource_callback(i64Timestamp_us, i32NoiseDiodeSource, strNoiseSource);
    }
}

void cStationControllerKATCPClient::sendNoiseDiodeCurrent(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeCurrent_callback(i64Timestamp_us, dNoiseDiodeCurrent_A);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeCurrent_callback(i64Timestamp_us, dNoiseDiodeCurrent_A);
    }
}

void cStationControllerKATCPClient::sendSourceSelection(int64_t i64Timestamp_us, const string &strSourceName, double dRighAscension_deg, double dDeclination_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->sourceSelection_callback(i64Timestamp_us, strSourceName, dRighAscension_deg, dDeclination_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->sourceSelection_callback(i64Timestamp_us, strSourceName, dRighAscension_deg, dDeclination_deg);
    }
}


void cStationControllerKATCPClient::sendFrequencyRF(int64_t i64Timestamp_us, double dFreqencyRF_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyRF_callback(i64Timestamp_us, dFreqencyRF_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyRF_callback(i64Timestamp_us, dFreqencyRF_MHz);
    }
}

void cStationControllerKATCPClient::sendFrequencyLOs(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, double dFrequencyLO2_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLOs_callback(i64Timestamp_us, dFrequencyLO1_MHz, dFrequencyLO2_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLOs_callback(i64Timestamp_us, dFrequencyLO1_MHz, dFrequencyLO2_MHz);
    }
}

void cStationControllerKATCPClient::sendBandwidthIF(int64_t i64Timestamp_us, double dBandwidthIF_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->bandwidthIF_callback(i64Timestamp_us, dBandwidthIF_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->bandwidthIF_callback(i64Timestamp_us, dBandwidthIF_MHz);
    }
}
