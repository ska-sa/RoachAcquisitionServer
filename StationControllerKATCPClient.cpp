
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
    // Antenna space
    m_vstrSensorSampling.push_back("acs.actual-azim period 1000");
    m_vstrSensorSampling.push_back("acs.actual-elev period 1000");
    m_vstrSensorSampling.push_back("acs.desired-azim period 1000");
    m_vstrSensorSampling.push_back("acs.desired-elev period 1000");
    m_vstrSensorSampling.push_back("acs.request-azim period 1000");
    m_vstrSensorSampling.push_back("acs.request-elev period 1000");

    //Sky space
    m_vstrSensorSampling.push_back("SCS.request-azim period 1000");
    m_vstrSensorSampling.push_back("SCS.request-elev period 1000");
    m_vstrSensorSampling.push_back("SCS.desired-azim period 1000");
    m_vstrSensorSampling.push_back("SCS.desired-elev period 1000");
    m_vstrSensorSampling.push_back("SCS.actual-azim period 1000");
    m_vstrSensorSampling.push_back("SCS.actual-elev period 1000");

    // Pointing model
    m_vstrSensorSampling.push_back("SCS.pmodel1 event");
    m_vstrSensorSampling.push_back("SCS.pmodel2 event");
    m_vstrSensorSampling.push_back("SCS.pmodel3 event");
    m_vstrSensorSampling.push_back("SCS.pmodel4 event");
    m_vstrSensorSampling.push_back("SCS.pmodel5 event");
    m_vstrSensorSampling.push_back("SCS.pmodel6 event");
    m_vstrSensorSampling.push_back("SCS.pmodel7 event");
    m_vstrSensorSampling.push_back("SCS.pmodel8 event");
    m_vstrSensorSampling.push_back("SCS.pmodel9 event");
    m_vstrSensorSampling.push_back("SCS.pmodel10 event");
    m_vstrSensorSampling.push_back("SCS.pmodel11 event");
    m_vstrSensorSampling.push_back("SCS.pmodel12 event");
    m_vstrSensorSampling.push_back("SCS.pmodel13 event");
    m_vstrSensorSampling.push_back("SCS.pmodel14 event");
    m_vstrSensorSampling.push_back("SCS.pmodel15 event");
    m_vstrSensorSampling.push_back("SCS.pmodel16 event");
    m_vstrSensorSampling.push_back("SCS.pmodel17 event");
    m_vstrSensorSampling.push_back("SCS.pmodel18 event");
    m_vstrSensorSampling.push_back("SCS.pmodel19 event");
    m_vstrSensorSampling.push_back("SCS.pmodel20 event");
    m_vstrSensorSampling.push_back("SCS.pmodel21 event");
    m_vstrSensorSampling.push_back("SCS.pmodel22 event");
    m_vstrSensorSampling.push_back("SCS.pmodel23 event");
    m_vstrSensorSampling.push_back("SCS.pmodel24 event");
    m_vstrSensorSampling.push_back("SCS.pmodel25 event");
    m_vstrSensorSampling.push_back("SCS.pmodel26 event");
    m_vstrSensorSampling.push_back("SCS.pmodel27 event");
    m_vstrSensorSampling.push_back("SCS.pmodel28 event");
    m_vstrSensorSampling.push_back("SCS.pmodel29 event");
    m_vstrSensorSampling.push_back("SCS.pmodel30 event");

    // Antenna status
    m_vstrSensorSampling.push_back("SCS.AntennaActivity event");
    m_vstrSensorSampling.push_back("SCS.Target event");

    // Receiver Mk2 values
    m_vstrSensorSampling.push_back("rx.fe.gain.band1-lcp");
    m_vstrSensorSampling.push_back("rx.fe.gain.band1-rcp");
    m_vstrSensorSampling.push_back("rx.fe.gain.band2-lcp");
    m_vstrSensorSampling.push_back("rx.fe.gain.band2-rcp");
    m_vstrSensorSampling.push_back("rx.fe.freq.band1");
    m_vstrSensorSampling.push_back("rx.fe.freq.band2");
    m_vstrSensorSampling.push_back("rx.fe.lcp-band-select");  // Done
    m_vstrSensorSampling.push_back("rx.fe.rcp-band-select");  // Done
    m_vstrSensorSampling.push_back("rx.stage1.status.noisediode.band1");
    m_vstrSensorSampling.push_back("rx.stage1.status.noisediode.band2");

    // Environment values.
    m_vstrSensorSampling.push_back("EMS.WindSpeed period 10000");
    m_vstrSensorSampling.push_back("EMS.WindDirection period 10000");
    m_vstrSensorSampling.push_back("EMS.AirTemperature period 10000");
    m_vstrSensorSampling.push_back("EMS.AbsolutePressure period 10000");
    m_vstrSensorSampling.push_back("EMS.RelativeHumidity period 10000");

    //TODO: Observation metadata such as targets, etc.
}

cStationControllerKATCPClient::~cStationControllerKATCPClient()
{
}

void cStationControllerKATCPClient::subscribeSensorData()
{
    cout << "cStationControllerKATCPClient::subscribeSensorData(): subscribing to sensor data." << endl;

    for (uint32_t ui = 0; ui < m_vstrSensorSampling.size(); ui++)
    {
        stringstream oSS;
        oSS << "?sensor-sampling ";
        oSS << m_vstrSensorSampling[ui];
        oSS << "\n";
        //oSS << " auto\n";

        sendKATCPMessage(oSS.str());
    }
}

void cStationControllerKATCPClient::unsubscribeSensorData()
{
    cout << "cStationControllerKATCPClient::unsubscribeSensorData(): unsubscribing from sensor data." << endl;

    for (uint32_t ui = 0; ui < m_vstrSensorSampling.size(); ui++)
    {
        stringstream oSS;
        oSS << "?sensor-sampling ";
        oSS << m_vstrSensorSampling[ui].substr(0,m_vstrSensorSampling[ui].find(" "));
        oSS << " none\n";

        sendKATCPMessage(oSS.str());
    }
}


void cStationControllerKATCPClient::onConnected()
{
    subscribeSensorData();
    return;
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
        if (!vstrTokens[1].compare("fail"))
        {
            cout << "cStationControllerKATCPClient::processKATCPMessage(): ";
            for (uint32_t ui = 0; ui < vstrTokens.size(); ui++)
            {
                cout << vstrTokens[ui] << " ";
            }
            cout << endl;
        }
        return;
    }

    if (!vstrTokens[0].compare("#log"))
    {
        // Do nothing.
        return;
    }

    if (!vstrTokens[0].compare("#client-connected"))
    {
        // Do nothing.
        return;
    }

    // All other known KATCP messages are 5 tokens long.
    // The *1e3 next to each timestamp is because the functions take timestamps in us
    // and katcp gives them in ms. The us thing was a decision by Craig a while ago,
    // and I'm too lazy to refactor it properly at this point. - James
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

    // Antenna space
    if( !vstrTokens[3].compare("acs.request-azim") )
    {
        sendAcsRequestedAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("acs.request-elev") )
    {
        sendAcsRequestedAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("acs.desired-azim") )
    {
        sendAcsDesiredAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("acs.desired-elev") )
    {
        sendAcsDesiredAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("acs.actual-azim"))
    {
        sendAcsActualAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("acs.actual-elev"))
    {
        sendAcsActualAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    // Sky space
    if( !vstrTokens[3].compare("SCS.request-azim") )
    {
        sendSkyRequestedAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("SCS.request-elev") )
    {
        sendSkyRequestedAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("SCS.desired-azim") )
    {
        sendSkyDesiredAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("SCS.desired-elev") )
    {
        sendSkyDesiredAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("SCS.actual-azim"))
    {
        sendSkyActualAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("SCS.actual-elev"))
    {
        sendSkyActualAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    // Pointing model
    if (    !vstrTokens[3].compare("SCS.pmodel1") ||
            !vstrTokens[3].compare("SCS.pmodel2") ||
            !vstrTokens[3].compare("SCS.pmodel3") ||
            !vstrTokens[3].compare("SCS.pmodel4") ||
            !vstrTokens[3].compare("SCS.pmodel5") ||
            !vstrTokens[3].compare("SCS.pmodel6") ||
            !vstrTokens[3].compare("SCS.pmodel7") ||
            !vstrTokens[3].compare("SCS.pmodel8") ||
            !vstrTokens[3].compare("SCS.pmodel9") ||
            !vstrTokens[3].compare("SCS.pmodel10") ||
            !vstrTokens[3].compare("SCS.pmodel11") ||
            !vstrTokens[3].compare("SCS.pmodel12") ||
            !vstrTokens[3].compare("SCS.pmodel13") ||
            !vstrTokens[3].compare("SCS.pmodel14") ||
            !vstrTokens[3].compare("SCS.pmodel15") ||
            !vstrTokens[3].compare("SCS.pmodel16") ||
            !vstrTokens[3].compare("SCS.pmodel17") ||
            !vstrTokens[3].compare("SCS.pmodel18") ||
            !vstrTokens[3].compare("SCS.pmodel19") ||
            !vstrTokens[3].compare("SCS.pmodel20") ||
            !vstrTokens[3].compare("SCS.pmodel21") ||
            !vstrTokens[3].compare("SCS.pmodel22") ||
            !vstrTokens[3].compare("SCS.pmodel23") ||
            !vstrTokens[3].compare("SCS.pmodel24") ||
            !vstrTokens[3].compare("SCS.pmodel25") ||
            !vstrTokens[3].compare("SCS.pmodel26") ||
            !vstrTokens[3].compare("SCS.pmodel27") ||
            !vstrTokens[3].compare("SCS.pmodel28") ||
            !vstrTokens[3].compare("SCS.pmodel29") ||
            !vstrTokens[3].compare("SCS.pmodel30")      )
    {
        sendPointingModelParameter( strtol(vstrTokens[3].substr(10,vstrTokens[3].size() - 10).c_str(), NULL, 10), strtod(vstrTokens[5].c_str(), NULL));
    }


    // Antenna status
    if(!vstrTokens[3].compare("SCS.AntennaActivity"))
    {
        sendAntennaStatus( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, vstrTokens[5].c_str(), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("SCS.Target"))
    {
        sendSourceSelection( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, vstrTokens[5].c_str(), vstrTokens[4].c_str()  );
        return;
    }

    // Receiver Mk 2
    if(!vstrTokens[3].compare("RFC.NoiseDiode_1"))
    {
        sendNoiseDiodeState( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtol(vstrTokens[5].c_str(), NULL, 10), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.lcp-band-select"))
    {
        // The latest SCS software doesn't just give us a 1 or 0, which is what
        // the data file expects, but a `band1` or `band2`, so we do a bit of
        // surgery to get it into the format preferred by the data file. Chop
        // off first four chars, convert to an int, subtract 1 and cast to bool.
        bool bBandSelect = (bool) strtol(vstrTokens[5].c_str() + 4, NULL, 10) - 1;
        sendBandSelectLcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, bBandSelect, vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.rcp-band-select"))
    {
        // see previous code block for explanation
        bool bBandSelect = (bool) strtol(vstrTokens[5].c_str() + 4, NULL, 10) - 1;
        sendBandSelectRcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, bBandSelect, vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.freq.band1"))
    {
        sendFrequencySky5GHz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.freq.band2"))
    {
        sendFrequencySky6_7GHz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("receiverBandwidthChan0"))
    {
        sendReceiverBandwidthLcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if(!vstrTokens[3].compare("receiverBandwidthChan1"))
    {
        sendReceiverBandwidthRcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("SCS.LcpAttenuation") )
    {
        sendReceiverLcpAttenuation( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("SCS.RcpAttenuation") )
    {
        sendReceiverRcpAttenuation( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    // Environment values
    if( !vstrTokens[3].compare("EMS.WindSpeed") )
    {
        sendWindSpeed( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("EMS.WindDirection") )
    {
        sendWindDirection( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("EMS.AirTemperature") )
    {
        sendTemperature( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("EMS.AbsolutePressure") )
    {
        sendAbsolutePressure( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("EMS.RelativeHumidity") )
    {
        sendRelativeHumidity( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
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

void  cStationControllerKATCPClient::sendStartRecording(const std::string &strFileSuffix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->startRecording_callback(strFileSuffix, i64StartTime_us, i64Duration_us);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->startRecording_callback(strFileSuffix, i64StartTime_us, i64Duration_us);
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

void cStationControllerKATCPClient::sendAcsRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->acsRequestedAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->acsRequestedAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendAcsRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->acsRequestedEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->acsRequestedEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendAcsDesiredAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->acsDesiredAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->acsDesiredAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendAcsDesiredAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->acsDesiredEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->acsDesiredEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendAcsActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->acsActualAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->acsActualAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendAcsActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->acsActualEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->acsActualEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}


void cStationControllerKATCPClient::sendSkyRequestedAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->skyRequestedAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->skyRequestedAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendSkyRequestedAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->skyRequestedEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->skyRequestedEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendSkyDesiredAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->skyDesiredAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->skyDesiredAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendSkyDesiredAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->skyDesiredEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->skyDesiredEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendSkyActualAntennaAz(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->skyActualAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->skyActualAz_callback(i64Timestamp_us, dAzimuth_deg, strStatus);
    }
}

void cStationControllerKATCPClient::sendSkyActualAntennaEl(int64_t i64Timestamp_us,double dElevation_deg, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->skyActualEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->skyActualEl_callback(i64Timestamp_us, dElevation_deg, strStatus);
    }
}



void cStationControllerKATCPClient::sendPointingModelParameter(uint8_t ui8ParameterNumber, double dParameterValue)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->pointingModel_callback(ui8ParameterNumber, dParameterValue);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->pointingModel_callback(ui8ParameterNumber, dParameterValue);
    }
}

void cStationControllerKATCPClient::sendAntennaStatus(int64_t i64Timestamp_us, std::string strAntennaStatus, const string &strStatus)
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


void cStationControllerKATCPClient::sendNoiseDiodeState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, const string &strStatus)
{
    /* NoiseDiodeState is a bitfield which needs to be unpacked.
     *  Bit       Description
     *  0 – 1     Input Source Select:
     *            00 – None
     *            01 – Roach
     *            10 – DBBC
     *            11 – PC
     *  2         N/A
     *  3         Enable
     *  4 – 7     Noise Diode Select:
     *            0001 – ND1
     *            0010 – ND2
     *            0100 – ND3
     *            1000 – ND4
     *  8 – 13    PWM Mark (0 – 63)
     *  14 – 15   Freq Select:
     *            00 – 0.5Hz
     *            01 – 1Hz
     *            10 – 10Hz
     *            11 – 200Hz
     */

    // For security. This should be only a 16-bit value.
    string strSource;
    switch (i32NoiseDiodeState & 0b00000000000000000000000000000011)
    {
        case 0: strSource = "None";
                break;
        case 1: strSource = "ROACH";
                break;
        case 2: strSource = "DBBC";
                break;
        case 3: strSource = "PC";
                break;
        default: strSource = "UNKNOWN";
                break;
    }

    bool bNoiseDiodeEnabled            = i32NoiseDiodeState & 0b00000000000000000000000000001000 >> 3;
    uint32_t i32NoiseDiodeSelect       = i32NoiseDiodeState & 0b00000000000000000000000011110000 >> 4;
    uint32_t i32NoiseDiodePWMMark      = i32NoiseDiodeState & 0b00000000000000000011111100000000 >> 8;
    double dNoiseDiodePWMFrequency;
    switch (i32NoiseDiodeState & 0b00000000000000001100000000000000 >> 14)
    {
    case 0: dNoiseDiodePWMFrequency = 0.5;
            break;
    case 1: dNoiseDiodePWMFrequency = 1.0;
            break;
    case 2: dNoiseDiodePWMFrequency = 10.0;
            break;
    case 3: dNoiseDiodePWMFrequency = 200.0;
            break;
    default: dNoiseDiodePWMFrequency = -1;
            break;
    }

    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->rNoiseDiodeInputSource_callback(i64Timestamp_us, strSource, strStatus);
        pHandler->rNoiseDiodeEnabled_callback(i64Timestamp_us, bNoiseDiodeEnabled, strStatus);
        pHandler->rNoiseDiodeSelect_callback(i64Timestamp_us, i32NoiseDiodeSelect, strStatus);
        pHandler->rNoiseDiodePWMMark_callback(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
        pHandler->rNoiseDiodePWMFrequency_callback(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->rNoiseDiodeInputSource_callback(i64Timestamp_us, strSource, strStatus);
        pHandler->rNoiseDiodeEnabled_callback(i64Timestamp_us, bNoiseDiodeEnabled, strStatus);
        pHandler->rNoiseDiodeSelect_callback(i64Timestamp_us, i32NoiseDiodeSelect, strStatus);
        pHandler->rNoiseDiodePWMMark_callback(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
        pHandler->rNoiseDiodePWMFrequency_callback(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
    }
}

void cStationControllerKATCPClient::sendSourceSelection(int64_t i64Timestamp_us, const string &strSourceName, const string &strStatus)
{
    // TODO: Cut \_ out of strSourceName, and pass along a string with spaces in it.
    vector<string> tokenisedSourceName = tokeniseString(strSourceName, string("\\_"));

    stringstream oSS;
    BOOST_FOREACH(string strWord, tokenisedSourceName)
    {
        oSS << strWord << " ";
    }

    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->sourceSelection_callback(i64Timestamp_us, oSS.str(), strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->sourceSelection_callback(i64Timestamp_us, oSS.str(), strStatus);
    }
}

void cStationControllerKATCPClient::sendBandSelectLcp(int64_t i64Timestamp_us, bool bFreqencyRFChan0_MHz, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->bandSelectLcp_callback(i64Timestamp_us, bFreqencyRFChan0_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->bandSelectLcp_callback(i64Timestamp_us, bFreqencyRFChan0_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendBandSelectRcp(int64_t i64Timestamp_us, bool bFreqencyRF_MHz, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->bandSelectRcp_callback
                (i64Timestamp_us, bFreqencyRF_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->bandSelectRcp_callback(i64Timestamp_us, bFreqencyRF_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendFrequencySky5GHz(int64_t i64Timestamp_us, double dFrequencySky5GHz_MHz, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencySky5GHz_callback(i64Timestamp_us, dFrequencySky5GHz_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencySky5GHz_callback(i64Timestamp_us, dFrequencySky5GHz_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendFrequencySky6_7GHz(int64_t i64Timestamp_us, double dFrequencySky6_7GHz_MHz, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencySky6_7GHz_callback(i64Timestamp_us, dFrequencySky6_7GHz_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencySky6_7GHz_callback(i64Timestamp_us, dFrequencySky6_7GHz_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverBandwidthLcp(int64_t i64Timestamp_us, double dReceiverBandwidthLcp_MHz, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverBandwidthLcp_callback(i64Timestamp_us, dReceiverBandwidthLcp_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverBandwidthLcp_callback(i64Timestamp_us, dReceiverBandwidthLcp_MHz, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverBandwidthRcp(int64_t i64Timestamp_us, double dReceiverBandwidthRcp_MHz, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverBandwidthRcp_callback(i64Timestamp_us, dReceiverBandwidthRcp_MHz, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverBandwidthRcp_callback(i64Timestamp_us, dReceiverBandwidthRcp_MHz, strStatus);
    }
}


void cStationControllerKATCPClient::sendReceiverLcpAttenuation(int64_t i64Timestamp_us, double dLcpAttenuation_dB, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverLcpAttenuation_callback(i64Timestamp_us, dLcpAttenuation_dB, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverLcpAttenuation_callback(i64Timestamp_us, dLcpAttenuation_dB, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverRcpAttenuation(int64_t i64Timestamp_us, double dRcpAttenuation_dB, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverRcpAttenuation_callback(i64Timestamp_us, dRcpAttenuation_dB, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverRcpAttenuation_callback(i64Timestamp_us, dRcpAttenuation_dB, strStatus);
    }
}

void cStationControllerKATCPClient::sendWindSpeed(int64_t i64Timestamp_us, double dWindSpeed_mps, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->envWindSpeed_callback(i64Timestamp_us, dWindSpeed_mps, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->envWindSpeed_callback(i64Timestamp_us, dWindSpeed_mps, strStatus);
    }
}

void cStationControllerKATCPClient::sendWindDirection(int64_t i64Timestamp_us, double dWindDirection_degrees, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->envWindDirection_callback(i64Timestamp_us, dWindDirection_degrees, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->envWindDirection_callback(i64Timestamp_us, dWindDirection_degrees, strStatus);
    }
}

void cStationControllerKATCPClient::sendTemperature(int64_t i64Timestamp_us, double dTemperature_degreesC, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->envTemperature_callback(i64Timestamp_us, dTemperature_degreesC, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->envTemperature_callback(i64Timestamp_us, dTemperature_degreesC, strStatus);
    }
}

void cStationControllerKATCPClient::sendAbsolutePressure(int64_t i64Timestamp_us, double dPressure_mbar, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->envAbsolutePressure_callback(i64Timestamp_us, dPressure_mbar, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->envAbsolutePressure_callback(i64Timestamp_us, dPressure_mbar, strStatus);
    }
}

void cStationControllerKATCPClient::sendRelativeHumidity(int64_t i64Timestamp_us, double dHumidity_percent, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->envRelativeHumidity_callback(i64Timestamp_us, dHumidity_percent, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->envRelativeHumidity_callback(i64Timestamp_us, dHumidity_percent, strStatus);
    }
}


// Don't remove - this should still be implemented in the station controller.
// Eventually.
/*
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

void cStationControllerKATCPClient::sendAntennaName(const string &strAntennaName)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaName_callback(strAntennaName);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaName_callback(strAntennaName);
    }
}

void cStationControllerKATCPClient::sendAntennaBeamwidth(const string &strAntennaBeamwidth)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaBeamwidth_callback(strAntennaBeamwidth);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaBeamwidth_callback(strAntennaBeamwidth);
    }
}

void cStationControllerKATCPClient::sendAntennaDelayModel(const string &strAntennaDelayModel)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaD(strAntennaName);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaName_callback(strAntennaName);
    }
}

void cStationControllerKATCPClient::sendAntennaDiameter(const string &strAntennaDiameter)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaDiameter_callback(strAntennaDiameter);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaDiameter_callback(strAntennaDiameter);
    }
}

void cStationControllerKATCPClient::sendAntennaLatitude(const string &strAntennaLatitude)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaLatitude_callback(strAntennaLatitude);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaLatitude_callback(strAntennaLatitude);
    }
}

void cStationControllerKATCPClient::sendAntennaLongitude(const string &strAntennaLongitude)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaLongitude_callback(strAntennaLongitude);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaLongitude_callback(strAntennaLongitude);
    }
}
*/
