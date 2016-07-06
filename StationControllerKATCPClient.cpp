
//System includes

//Library includes
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

//Local includes
#include "StationControllerKATCPClient.h"

using namespace std;

cStationControllerKATCPClient::cStationControllerKATCPClient() :
    cKATCPClientBase()
{
    m_strAntennaStatus = "";
    m_strAppliedPointingModel = "";
    m_strSourceSelection = "";
}

cStationControllerKATCPClient::~cStationControllerKATCPClient()
{
}

void cStationControllerKATCPClient::subscribeSensorData()
{
    //TODO: This doesn't seem the best way to do things, probably should be private members or something.
    cout << "cStationControllerKATCPClient::subscribeSensorData(): subscribing to sensor data." << endl;
    vector<string> vstrSensorNames;
    vstrSensorNames.push_back("requestedAntennaAz");
    vstrSensorNames.push_back("requestedAntennaEl");
    vstrSensorNames.push_back("actualAntennaAz");
    vstrSensorNames.push_back("actualAntennaEl");
    vstrSensorNames.push_back("actualSourceOffsetAz");
    vstrSensorNames.push_back("actualSourceOffsetEl");
    vstrSensorNames.push_back("actualAntennaRA");
    vstrSensorNames.push_back("actualAntennaDec");
    vstrSensorNames.push_back("antennaStatus");
    vstrSensorNames.push_back("motorTorqueAzMaster");
    vstrSensorNames.push_back("motorTorqueAzSlave");
    vstrSensorNames.push_back("motorTorqueElMaster");
    vstrSensorNames.push_back("motorTorqueElSlave");
    vstrSensorNames.push_back("appliedPointingModel");
    vstrSensorNames.push_back("noiseDiodeSoftwareState");
    vstrSensorNames.push_back("noiseDiodeSource");
    vstrSensorNames.push_back("noideDiodeCurrent");
    vstrSensorNames.push_back("sourceSelection");
    vstrSensorNames.push_back("frequencyRFChan0");
    vstrSensorNames.push_back("frequencyRFChan1");
    vstrSensorNames.push_back("frequencyLO0RFChan0");
    vstrSensorNames.push_back("frequencyLO0RFChan1");
    vstrSensorNames.push_back("frequencyLO1");
    vstrSensorNames.push_back("receiverBandwidthChan0");
    vstrSensorNames.push_back("receiverBandwidthChan1");

    for (uint32_t ui = 0; ui < vstrSensorNames.size(); ui++)
    {
        stringstream oSS;
        oSS << "?sensor-sampling ";
        oSS << vstrSensorNames[ui];
        oSS << " auto\n";

        sendKATCPMessage(oSS.str());
    }
}


void cStationControllerKATCPClient::unsubscribeSensorData()
{
    cout << "cStationControllerKATCPClient::unsubscribeSensorData(): unsubscribing from sensor data." << endl;
    vector<string> vstrSensorNames;
    vstrSensorNames.push_back("requestedAntennaAz");
    vstrSensorNames.push_back("requestedAntennaEl");
    vstrSensorNames.push_back("actualAntennaAz");
    vstrSensorNames.push_back("actualAntennaEl");
    vstrSensorNames.push_back("actualSourceOffsetAz");
    vstrSensorNames.push_back("actualSourceOffsetEl");
    vstrSensorNames.push_back("actualAntennaRA");
    vstrSensorNames.push_back("actualAntennaDec");
    vstrSensorNames.push_back("antennaStatus");
    vstrSensorNames.push_back("motorTorqueAzMaster");
    vstrSensorNames.push_back("motorTorqueAzSlave");
    vstrSensorNames.push_back("motorTorqueElMaster");
    vstrSensorNames.push_back("motorTorqueElSlave");
    vstrSensorNames.push_back("appliedPointingModel");
    vstrSensorNames.push_back("noiseDiodeSoftwareState");
    vstrSensorNames.push_back("noiseDiodeSource");
    vstrSensorNames.push_back("noideDiodeCurrent");
    vstrSensorNames.push_back("sourceSelection");
    vstrSensorNames.push_back("frequencyRFChan0");
    vstrSensorNames.push_back("frequencyRFChan1");
    vstrSensorNames.push_back("frequencyLO0RFChan0");
    vstrSensorNames.push_back("frequencyLO0RFChan1");
    vstrSensorNames.push_back("frequencyLO1");
    vstrSensorNames.push_back("receiverBandwidthChan0");
    vstrSensorNames.push_back("receiverBandwidthChan1");

    for (uint32_t ui = 0; ui < vstrSensorNames.size(); ui++)
    {
        stringstream oSS;
        oSS << "?sensor-sampling ";
        oSS << vstrSensorNames[ui];
        oSS << " none\n";

        sendKATCPMessage(oSS.str());
    }
}


void cStationControllerKATCPClient::onConnected()
{
    return; // Don't have anything to do here right at this moment.
}

void cStationControllerKATCPClient::processKATCPMessage(const vector<string> &vstrTokens)
{

    //Debug
    /*cout << "cStationControllerKATCPClient::processKATCPMessage(): KATCP message received: " << endl;
    for (uint32_t ui = 0; ui < vstrTokens.size(); ui++)
    {
     cout << vstrTokens[ui] << " ";
    }
    cout << endl;
    //*/

    // This must come first!
    // The katcp server on the station controller sends #version and #build-state tokens on connection.
    // If they get through to the next tests, then the program segfaults because
    if( !vstrTokens[0].compare("#version") )
    {
        printVersion(vstrTokens[1]);
        return;
    }

    if( !vstrTokens[0].compare("#build-state") )
    {
        printBuildState(vstrTokens[1]);
        return;
    }

    if (!vstrTokens[0].compare("!sensor-sampling"))
    {
        // Do nothing, this is just the inform from the server.
        return;
    }

    if (!vstrTokens[0].compare("#log"))
    {
        // Do nothing.
        return;
    }

    if( !vstrTokens[0].compare("#startRecording") )
    {
        // I'm commenting this out for now, so that NQ and I can determine whether this is in u-seconds or not. - JNS
        //sendStartRecording(vstrTokens[1], strtoll(vstrTokens[2].c_str(), NULL, 10)*1e3 , strtoll(vstrTokens[3].c_str(), NULL, 10)*1e3 ); // *1e3 to convert from ms to us.
        subscribeSensorData();
        return;
    }

    if( !vstrTokens[0].compare("#stopRecording") )
    {
        sendStopRecording();
        unsubscribeSensorData();
        return;
    }

    if( !vstrTokens[0].compare("#antennaStatus") )
    {
        cout << "cStationControllerKATCPClient::processKATCPMessage: Antenna status received: " << vstrTokens[1] << endl;
        m_strAntennaStatus = vstrTokens[1];
        return;
    }

    if( !vstrTokens[0].compare("#appliedPointingModel") )
    {
        m_strPointingModelName    = vstrTokens[1];
        m_strAppliedPointingModel = vstrTokens[2];
        return;
    }

    if( !vstrTokens[0].compare("#sourceSelection") )
    {
        m_strSourceSelection = vstrTokens[1];
        return;
    }

    // All other known KATCP messages are 5 tokens long.
    if(vstrTokens.size() < 5)
    {
        cout << "cStationControllerKATCPClient::processKATCPMessage(): Error! Unknown KATCP message received: ";
        for (uint32_t ui = 0; ui < vstrTokens.size(); ui++)
        {
            cout << vstrTokens[ui] << " ";
        }
        cout << endl;
        return;
    }


    if( !vstrTokens[3].compare("requestedAntennaAz") )
    {
        sendRequestedAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if( !vstrTokens[3].compare("requestedAntennaEl") )
    {
        sendRequestedAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("actualAntennaAz"))
    {
        sendActualAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("actualAntennaEl"))
    {
        sendActualAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("actualSourceOffsetAz"))
    {
        sendActualSourceOffsetAz( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("actualSourceOffsetEl"))
    {
        sendActualSourceOffsetEl( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("actualAntennaRA"))
    {
        sendActualAntennaRA( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("actualAntennaDec"))
    {
        sendActualAntennaDec( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("antennaStatus"))
    {
        cout << "cStationControllerKATCPClient::processKATCPMessage: Antenna Status command received: " << m_strAntennaStatus << endl;
        sendAntennaStatus( strtoll(vstrTokens[1].c_str(), NULL, 10), m_strAntennaStatus, vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("motorTorqueAzMaster"))
    {
        sendMotorTorqueAzMaster( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }



    if(!vstrTokens[3].compare("motorTorqueAzSlave"))
    {
        sendMotorTorqueAzSlave( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("motorTorqueElMaster"))
    {
        sendMotorTorqueElMaster( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("motorTorqueElSlave"))
    {
        sendMotorTorqueElSlave( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("appliedPointingModel"))
    {
        //Tokenise the pointing model string.
        string strPointingModelTokens = m_strAppliedPointingModel;
        boost::char_separator<char> separator(",");
        boost::tokenizer< boost::char_separator<char> > tokens(strPointingModelTokens, separator);

        vector<double> vdPointingModel;

        BOOST_FOREACH (const string& t, tokens)
        {
            vdPointingModel.push_back(strtod(t.c_str(), NULL));
        }

        sendAppliedPointingModel( m_strPointingModelName, vdPointingModel );
        return;
    }


    if(!vstrTokens[3].compare("noiseDiodeSoftwareState"))
    {
        sendNoiseDiodeSoftwareState( strtoll(vstrTokens[1].c_str(), NULL, 10), strtol(vstrTokens[2].c_str(), NULL, 10), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("noiseDiodeSource"))
    {
        sendNoiseDiodeSource( strtoll(vstrTokens[1].c_str(), NULL, 10), vstrTokens[3], vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("noideDiodeCurrent"))
    {
        sendNoiseDiodeCurrent( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("sourceSelection"))
    {
        //TODO: Process this one. Doesn't have any other values as yet. Look at how MeerKAT does it. How does KatDAL get this data from the catalog?
        //sendSourceSelection( strtoll(vstrTokens[1].c_str(), NULL, 10), vstrTokens[2], strtod(vstrTokens[3].c_str(), NULL), strtod(vstrTokens[4].c_str(), NULL) );
        return;
    }


    if(!vstrTokens[3].compare("frequencyRFChan0"))
    {
        sendFrequencyRFChan0( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }



    if(!vstrTokens[3].compare("frequencyRFChan1"))
    {
        sendFrequencyRFChan1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }



    if(!vstrTokens[3].compare("frequencyLO0RFChan0"))
    {
        sendFrequencyLO0Chan0( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("frequencyLO0RFChan1"))
    {
        sendFrequencyLO0Chan1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("frequencyLO1"))
    {
        sendFrequencyLO1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }


    if(!vstrTokens[3].compare("receiverBandwidthChan0"))
    {
        sendReceiverBandwidthChan0( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }



    if(!vstrTokens[3].compare("receiverBandwidthChan1"))
    {
        sendReceiverBandwidthChan1( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

}

void cStationControllerKATCPClient::printVersion(const std::string &strVersion)
{
    cout << "cStationControllerKATCPClient::processKATCPMessage(): KATCP client connected to Station Controller, version " << strVersion << endl;
}

void cStationControllerKATCPClient::printBuildState(const std::string &strBuildState)
{
    cout << "cStationControllerKATCPClient::processKATCPMessage(): Station Controller build state: " << strBuildState << endl;
}

void  cStationControllerKATCPClient::sendStartRecording(const std::string &strFilePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

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
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

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

void cStationControllerKATCPClient::sendRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->requestedAntennaAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->requestedAntennaAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->requestedAntennaEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->requestedAntennaEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendActualSourceOffsetAz(int64_t i64Timestamp_us, double dAzimuthOffset_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualSourceOffsetAz_callback(i64Timestamp_us, dAzimuthOffset_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualSourceOffsetAz_callback(i64Timestamp_us, dAzimuthOffset_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendActualSourceOffsetEl(int64_t i64Timestamp_us, double dElevationOffset_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualSourceOffsetEl_callback(i64Timestamp_us, dElevationOffset_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualSourceOffsetEl_callback(i64Timestamp_us, dElevationOffset_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendActualAntennaRA(int64_t i64Timestamp_us, double dRighAscension_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaRA_callback(i64Timestamp_us, dRighAscension_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaRA_callback(i64Timestamp_us, dRighAscension_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendActualAntennaDec(int64_t i64Timestamp_us, double dDeclination_deg, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->actualAntennaDec_callback(i64Timestamp_us, dDeclination_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->actualAntennaDec_callback(i64Timestamp_us, dDeclination_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendAntennaStatus(int64_t i64Timestamp_us, std::string strAntennaStatus, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaStatus_callback(i64Timestamp_us, strAntennaStatus, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaStatus_callback(i64Timestamp_us, strAntennaStatus, strStatus);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueAzMaster(int64_t i64Timestamp_us, double dAzMaster_nNm, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueAzMaster_callback(i64Timestamp_us, dAzMaster_nNm, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueAzMaster_callback(i64Timestamp_us, dAzMaster_nNm, strStatus);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueAzSlave(int64_t i64Timestamp_us, double dAzSlave_nNm, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueAzSlave_callback(i64Timestamp_us, dAzSlave_nNm, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueAzSlave_callback(i64Timestamp_us, dAzSlave_nNm, strStatus);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueElMaster(int64_t i64Timestamp_us, double dElMaster_nNm, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueElMaster_callback(i64Timestamp_us, dElMaster_nNm, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueElMaster_callback(i64Timestamp_us, dElMaster_nNm, strStatus);
    }
}

void cStationControllerKATCPClient::sendMotorTorqueElSlave(int64_t i64Timestamp_us, double dElSlave_nNm, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->motorTorqueElSlave_callback(i64Timestamp_us, dElSlave_nNm, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->motorTorqueElSlave_callback(i64Timestamp_us, dElSlave_nNm, strStatus);
    }
}

void cStationControllerKATCPClient::sendAppliedPointingModel(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

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

void cStationControllerKATCPClient::sendNoiseDiodeSoftwareState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeSoftwareState_callback(i64Timestamp_us, i32NoiseDiodeState, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeSoftwareState_callback(i64Timestamp_us, i32NoiseDiodeState, strStatus);
    }
}

void cStationControllerKATCPClient::sendNoiseDiodeSource(int64_t i64Timestamp_us, const string &strNoiseSource, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeSource_callback(i64Timestamp_us, strNoiseSource, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeSource_callback(i64Timestamp_us, strNoiseSource, strStatus);
    }
}

void cStationControllerKATCPClient::sendNoiseDiodeCurrent(int64_t i64Timestamp_us, double dNoiseDiodeCurrent_A, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeCurrent_callback(i64Timestamp_us, dNoiseDiodeCurrent_A, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeCurrent_callback(i64Timestamp_us, dNoiseDiodeCurrent_A, strStatus);
    }
}

void cStationControllerKATCPClient::sendSourceSelection(int64_t i64Timestamp_us, const string &strSourceName, double dRighAscension_deg, double dDeclination_deg)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

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


void cStationControllerKATCPClient::sendFrequencyRFChan0(int64_t i64Timestamp_us, double dFreqencyRFChan0_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyRFChan0_callback(i64Timestamp_us, dFreqencyRFChan0_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyRFChan0_callback(i64Timestamp_us, dFreqencyRFChan0_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendFrequencyRFChan1(int64_t i64Timestamp_us, double dFreqencyRFChan1_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyRFChan1_callback(i64Timestamp_us, dFreqencyRFChan1_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyRFChan1_callback(i64Timestamp_us, dFreqencyRFChan1_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendFrequencyLO0Chan0(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLO0Chan0_callback(i64Timestamp_us, dFrequencyLO0Chan0_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLO0Chan0_callback(i64Timestamp_us, dFrequencyLO0Chan0_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendFrequencyLO0Chan1(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLO0Chan1_callback(i64Timestamp_us, dFrequencyLO0Chan1_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLO0Chan1_callback(i64Timestamp_us, dFrequencyLO0Chan1_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendFrequencyLO1(int64_t i64Timestamp_us, double dFrequencyLO1_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyLO1_callback(i64Timestamp_us, dFrequencyLO1_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyLO1_callback(i64Timestamp_us, dFrequencyLO1_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverBandwidthChan0(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverBandwidthChan0_callback(i64Timestamp_us, dReceiverBandwidthChan0_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverBandwidthChan0_callback(i64Timestamp_us, dReceiverBandwidthChan0_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverBandwidthChan1(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_MHz, const string strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverBandwidthChan1_callback(i64Timestamp_us, dReceiverBandwidthChan1_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverBandwidthChan1_callback(i64Timestamp_us, dReceiverBandwidthChan1_MHz, strStatus);
    }
}
