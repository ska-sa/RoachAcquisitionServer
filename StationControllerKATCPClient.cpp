
//System includes

//Library includes

//Local includes
#include "StationControllerKATCPClient.h"

using namespace std;

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
        if( !vstrTokens[0].compare("#startRecording") )
        {
            sendStartRecording(vstrTokens[1], strtoll(vstrTokens[2].c_str(), NULL, 10), strtoll(vstrTokens[3].c_str(), NULL, 10) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if( !vstrTokens[0].compare("#stopRecording") )
        {
            sendStopRecording();
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if( !vstrTokens[0].compare("#requestedAntennaAz") )
        {
            sendRequestedAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if( !vstrTokens[0].compare("#requestedAntennaEl") )
        {
            sendRequestedAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualAntennaAz"))
        {
            sendActualAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualAntennaAz"))
        {
            sendActualAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualSourceOffsetAz"))
        {
            sendActualSourceOffsetAz( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualSourceOffsetEl"))
        {
            sendActualSourceOffsetEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualAntennaRA"))
        {
            sendActualAntennaRA( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#actualAntennaDec"))
        {
            sendActualAntennaDec( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
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
            sendAntennaStatus( strtoll(vstrTokens[1].c_str(), NULL, 10), vstrTokens[2] );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#motorTorqueAzMaster"))
        {
            sendMotorTorqueAzMaster( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#motorTorqueAzSlave"))
        {
            sendMotorTorqueAzSlave( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#motorTorqueElMaster"))
        {
            sendMotorTorqueElMaster( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#motorTorqueElSlave"))
        {
            sendMotorTorqueElSlave( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
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
            sendNoiseDiodeSource( strtoll(vstrTokens[1].c_str(), NULL, 10), vstrTokens[3] );
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
        if(!vstrTokens[0].compare("#frequencyRFChan0"))
        {
            sendFrequencyRFChan0( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyRFChan1"))
        {
            sendFrequencyRFChan1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyLO0Chan0"))
        {
            sendFrequencyLO0Chan0( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyLO0Chan1"))
        {
            sendFrequencyLO0Chan1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyLO1"))
        {
            sendFrequencyLO1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#receiverBandwidthChan0"))
        {
            sendReceiverBandwidthChan0( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#receiverBandwidthChan1"))
        {
            sendReceiverBandwidthChan1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL) );
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

void cStationControllerKATCPClient::sendRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->requestedAntennaAz_callback(i64Timestamp_us, dAzimuth_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->requestedAntennaAz_callback(i64Timestamp_us, dAzimuth_deg);
    }
}

void cStationControllerKATCPClient::sendRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->requestedAntennaEl_callback(i64Timestamp_us, dElevation_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->requestedAntennaEl_callback(i64Timestamp_us, dElevation_deg);
    }
}

void cStationControllerKATCPClient::sendActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaAz_callback(i64Timestamp_us, dAzimuth_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaAz_callback(i64Timestamp_us, dAzimuth_deg);
    }
}

void cStationControllerKATCPClient::sendActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaEl_callback(i64Timestamp_us, dElevation_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaEl_callback(i64Timestamp_us, dElevation_deg);
    }
}

void cStationControllerKATCPClient::sendActualSourceOffsetAz(int64_t i64Timestamp_us, double dAzimuthOffset_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualSourceOffsetAz_callback(i64Timestamp_us, dAzimuthOffset_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualSourceOffsetAz_callback(i64Timestamp_us, dAzimuthOffset_deg);
    }
}

void cStationControllerKATCPClient::sendActualSourceOffsetEl(int64_t i64Timestamp_us, double dElevationOffset_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualSourceOffsetEl_callback(i64Timestamp_us, dElevationOffset_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualSourceOffsetEl_callback(i64Timestamp_us, dElevationOffset_deg);
    }
}

void cStationControllerKATCPClient::sendActualAntennaRA(int64_t i64Timestamp_us, double dRighAscension_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaRA_callback(i64Timestamp_us, dRighAscension_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaRA_callback(i64Timestamp_us, dRighAscension_deg);
    }
}

void cStationControllerKATCPClient::sendActualAntennaDec(int64_t i64Timestamp_us, double dDeclination_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaDec_callback(i64Timestamp_us, dDeclination_deg);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaDec_callback(i64Timestamp_us, dDeclination_deg);
    }
}

void cStationControllerKATCPClient::sendAntennaStatus(int64_t i64Timestamp_us, std::string strAntennaStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaStatus_callback(i64Timestamp_us, strAntennaStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaStatus_callback(i64Timestamp_us, strAntennaStatus);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueAzMaster(int64_t i64Timestamp_us, double dAzMaster_nNm)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueAzMaster_callback(i64Timestamp_us, dAzMaster_nNm);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueAzMaster_callback(i64Timestamp_us, dAzMaster_nNm);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueAzSlave(int64_t i64Timestamp_us, double dAzSlave_nNm)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueAzSlave_callback(i64Timestamp_us, dAzSlave_nNm);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueAzSlave_callback(i64Timestamp_us, dAzSlave_nNm);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueElMaster(int64_t i64Timestamp_us, double dElMaster_nNm)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueElMaster_callback(i64Timestamp_us, dElMaster_nNm);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueElMaster_callback(i64Timestamp_us, dElMaster_nNm);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueElSlave(int64_t i64Timestamp_us, double dElSlave_nNm)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueElSlave_callback(i64Timestamp_us, dElSlave_nNm);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueElSlave_callback(i64Timestamp_us, dElSlave_nNm);
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

void cStationControllerKATCPClient::sendNoiseDiodeSource(int64_t i64Timestamp_us, const string &strNoiseSource)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeSource_callback(i64Timestamp_us, strNoiseSource);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeSource_callback(i64Timestamp_us, strNoiseSource);
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


void cStationControllerKATCPClient::sendFrequencyRFChan0(int64_t i64Timestamp_us, double dFreqencyRFChan0_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyRFChan0_callback(i64Timestamp_us, dFreqencyRFChan0_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyRFChan0_callback(i64Timestamp_us, dFreqencyRFChan0_MHz);
    }
}

void cStationControllerKATCPClient::sendFrequencyRFChan1(int64_t i64Timestamp_us, double dFreqencyRFChan1_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyRFChan1_callback(i64Timestamp_us, dFreqencyRFChan1_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyRFChan1_callback(i64Timestamp_us, dFreqencyRFChan1_MHz);
    }
}

void cStationControllerKATCPClient::sendFrequencyLO0Chan0(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLO0Chan0_callback(i64Timestamp_us, dFrequencyLO0Chan0_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLO0Chan0_callback(i64Timestamp_us, dFrequencyLO0Chan0_MHz);
    }
}

void cStationControllerKATCPClient::sendFrequencyLO0Chan1(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLO0Chan1_callback(i64Timestamp_us, dFrequencyLO0Chan1_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLO0Chan1_callback(i64Timestamp_us, dFrequencyLO0Chan1_MHz);
    }
}

void cStationControllerKATCPClient::sendFrequencyLO1(int64_t i64Timestamp_us, double dFrequencyLO1_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLO1_callback(i64Timestamp_us, dFrequencyLO1_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLO1_callback(i64Timestamp_us, dFrequencyLO1_MHz);
    }
}

void cStationControllerKATCPClient::sendReceiverBandwidthChan0(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverBandwidthChan0_callback(i64Timestamp_us, dReceiverBandwidthChan0_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverBandwidthChan0_callback(i64Timestamp_us, dReceiverBandwidthChan0_MHz);
    }
}

void cStationControllerKATCPClient::sendReceiverBandwidthChan1(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverBandwidthChan1_callback(i64Timestamp_us, dReceiverBandwidthChan1_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverBandwidthChan1_callback(i64Timestamp_us, dReceiverBandwidthChan1_MHz);
    }
}
