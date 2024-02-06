
//System includes
#include <iostream>

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
    m_vstrSensorSampling.push_back("acs.actual-azim period 1");
    m_vstrSensorSampling.push_back("acs.actual-elev period 1");
    m_vstrSensorSampling.push_back("acs.desired-azim period 1");
    m_vstrSensorSampling.push_back("acs.desired-elev period 1");
    m_vstrSensorSampling.push_back("acs.request-azim period 1");
    m_vstrSensorSampling.push_back("acs.request-elev period 1");

    //Sky space
    m_vstrSensorSampling.push_back("SCS.request-azim period 1");
    m_vstrSensorSampling.push_back("SCS.request-elev period 1");
    m_vstrSensorSampling.push_back("SCS.desired-azim period 1");
    m_vstrSensorSampling.push_back("SCS.desired-elev period 1");
    m_vstrSensorSampling.push_back("SCS.actual-azim period 1");
    m_vstrSensorSampling.push_back("SCS.actual-elev period 1");

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
    m_vstrSensorSampling.push_back("rx.fe.gain.band1-lcp event");
    m_vstrSensorSampling.push_back("rx.fe.gain.band1-rcp event");
    m_vstrSensorSampling.push_back("rx.fe.gain.band2-lcp event");
    m_vstrSensorSampling.push_back("rx.fe.gain.band2-rcp event");
    m_vstrSensorSampling.push_back("rx.fe.freq.band1 event");
    m_vstrSensorSampling.push_back("rx.fe.freq.band2 event");
    m_vstrSensorSampling.push_back("rx.fe.lcp-band-select event");
    m_vstrSensorSampling.push_back("rx.fe.rcp-band-select event");
    m_vstrSensorSampling.push_back("rx.stage1.status.noisediode.band1 event");
    m_vstrSensorSampling.push_back("rx.stage1.status.noisediode.band2 event");

    // Environment values.
    m_vstrSensorSampling.push_back("EMS.Logger.WindSpeed period 10");
    m_vstrSensorSampling.push_back("EMS.Logger.WindDirection period 10");
    m_vstrSensorSampling.push_back("EMS.Met4.Temperature period 10");
    m_vstrSensorSampling.push_back("EMS.Met4.AbsolutePressure period 10");
    m_vstrSensorSampling.push_back("EMS.Met4.RelativeHumidity period 10");

    // Observation metadata such as targets, etc.
    m_vstrSensorSampling.push_back("SCS.HPBW event");
    m_vstrSensorSampling.push_back("SCS.ObservationInformation event");
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

#ifdef __KATCP_V4
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
#else
    if( !vstrTokens[0].compare("#version-connect") )
    {
        if( !vstrTokens[1].compare("katcp-device") )
        {
            printVersion(vstrTokens[2]);
        }
        return;
    }

#endif

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

    /* If something changes on the SCS, like a component going down for instance,
     * and then coming back up, we may lose some metadata. So any time the
     * KATCP interface changes, we re-subscribe to all the sensors.
     */
    if (!vstrTokens[0].compare("#interface-changed"))
    {
        subscribeSensorData();
        return;
    }

    // All other known KATCP messages are 5 tokens long.
    // The *1e3 / * 1e6 next to each timestamp is because the functions take timestamps in us
    // and katcp v4 gives them in ms and katcp v5 in seconds. The us thing was a decision by Craig a while ago,
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
#ifdef __KATCP_V4
        sendAcsRequestedAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAcsRequestedAntennaAz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("acs.request-elev") )
    {
#ifdef __KATCP_V4
        sendAcsRequestedAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAcsRequestedAntennaEl( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("acs.desired-azim") )
    {
#ifdef __KATCP_V4
        sendAcsDesiredAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAcsDesiredAntennaAz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("acs.desired-elev") )
    {
#ifdef __KATCP_V4
        sendAcsDesiredAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAcsDesiredAntennaEl( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("acs.actual-azim"))
    {
#ifdef __KATCP_V4
        sendAcsActualAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAcsActualAntennaAz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("acs.actual-elev"))
    {
#ifdef __KATCP_V4
        sendAcsActualAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAcsActualAntennaEl( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    // Sky space
    if( !vstrTokens[3].compare("SCS.request-azim") )
    {
#ifdef __KATCP_V4
        sendSkyRequestedAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendSkyRequestedAntennaAz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("SCS.request-elev") )
    {
#ifdef __KATCP_V4
        sendSkyRequestedAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendSkyRequestedAntennaEl( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("SCS.desired-azim") )
    {
#ifdef __KATCP_V4
        sendSkyDesiredAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendSkyDesiredAntennaAz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("SCS.desired-elev") )
    {
#ifdef __KATCP_V4
        sendSkyDesiredAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendSkyDesiredAntennaEl( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("SCS.actual-azim"))
    {
#ifdef __KATCP_V4
        sendSkyActualAntennaAz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendSkyActualAntennaAz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("SCS.actual-elev"))
    {
#ifdef __KATCP_V4
        sendSkyActualAntennaEl( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendSkyActualAntennaEl( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
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
#ifdef __KATCP_V4
        sendAntennaStatus( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, vstrTokens[5].c_str(), vstrTokens[4].c_str() );
#else
        sendAntennaStatus( strtod(vstrTokens[1].c_str(), NULL)*1e6, vstrTokens[5].c_str(), vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("SCS.Target"))
    {
#ifdef __KATCP_V4
        sendSourceSelection( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, vstrTokens[5].c_str(), vstrTokens[4].c_str()  );
#else
        sendSourceSelection( strtod(vstrTokens[1].c_str(), NULL)*1e6, vstrTokens[5].c_str(), vstrTokens[4].c_str()  );
#endif
        return;
    }

    // Receiver Mk 2
    if(!vstrTokens[3].compare("rx.stage1.status.noisediode.band1") || !vstrTokens[3].compare("rx.stage1.status.noisediode.band2"))
    {
        // Similar surgery to the band-select functions below, because I'm not
        // horribly keen at this stage to type out this function twice but with
        // basically one character's difference.
        bool bNoiseDiodeSelect = (bool) (strtol(vstrTokens[3].c_str() + 32, NULL, 10) - 1);
#ifdef __KATCP_V4
        sendNoiseDiodeState( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtol(vstrTokens[5].c_str(), NULL, 10), bNoiseDiodeSelect, vstrTokens[4].c_str() );
#else
        sendNoiseDiodeState( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtol(vstrTokens[5].c_str(), NULL, 10), bNoiseDiodeSelect, vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.lcp-band-select"))
    {
        // The latest SCS software doesn't just give us a 1 or 0, which is what
        // the data file expects, but a `band1` or `band2`, so we do a bit of
        // surgery to get it into the format preferred by the data file.
        bool bBandSelect = (bool) (strtol(vstrTokens[5].c_str() + 4, NULL, 10) - 1);
#ifdef __KATCP_V4
        sendBandSelectLcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, bBandSelect, vstrTokens[4].c_str() );
#else
        sendBandSelectLcp( strtod(vstrTokens[1].c_str(), NULL)*1e6, bBandSelect, vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.rcp-band-select"))
    {
        // see previous code block for explanation
        bool bBandSelect = (bool) (strtol(vstrTokens[5].c_str() + 4, NULL, 10) - 1);
#ifdef __KATCP_V4
        sendBandSelectRcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, bBandSelect, vstrTokens[4].c_str() );
#else
        sendBandSelectRcp( strtod(vstrTokens[1].c_str(), NULL)*1e6, bBandSelect, vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.freq.band1"))
    {
#ifdef __KATCP_V4
        sendFrequencySky5GHz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendFrequencySky5GHz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if(!vstrTokens[3].compare("rx.fe.freq.band2"))
    {
#ifdef __KATCP_V4
        sendFrequencySky6_7GHz( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendFrequencySky6_7GHz( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("rx.fe.gain.band1-lcp") )
    {
#ifdef __KATCP_V4
        sendReceiverGain5GHzLcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendReceiverGain5GHzLcp( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("rx.fe.gain.band1-rcp") )
    {
#ifdef __KATCP_V4
        sendReceiverGain5GHzRcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendReceiverGain5GHzRcp( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("rx.fe.gain.band2-lcp") )
    {
#ifdef __KATCP_V4
        sendReceiverGain6_7GHzLcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendReceiverGain6_7GHzLcp( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("rx.fe.gain.band2-rcp") )
    {
#ifdef __KATCP_V4
        sendReceiverGain6_7GHzRcp( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendReceiverGain6_7GHzRcp( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    // Environment values
    if( !vstrTokens[3].compare("EMS.Logger.WindSpeed") )
    {
#ifdef __KATCP_V4
        sendWindSpeed( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendWindSpeed( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("EMS.Logger.WindDirection") )
    {
#ifdef __KATCP_V4
        sendWindDirection( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendWindDirection( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("EMS.Met4.Temperature") )
    {
#ifdef __KATCP_V4
        sendTemperature( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendTemperature( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("EMS.Met4.AbsolutePressure") )
    {
#ifdef __KATCP_V4
        sendAbsolutePressure( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendAbsolutePressure( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

    if( !vstrTokens[3].compare("EMS.Met4.RelativeHumidity") )
    {
#ifdef __KATCP_V4
        sendRelativeHumidity( strtoll(vstrTokens[1].c_str(), NULL, 10)*1e3, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#else
        sendRelativeHumidity( strtod(vstrTokens[1].c_str(), NULL)*1e6, strtod(vstrTokens[5].c_str(), NULL), vstrTokens[4].c_str() );
#endif
        return;
    }

// PJP
    if( !vstrTokens[3].compare("SCS.HPBW") )
    {
        sendAntennaBeamwidth( strtod(vstrTokens[1].c_str(), NULL)*1e6, vstrTokens[5].c_str(), vstrTokens[4].c_str() );
        return;
    }

    if( !vstrTokens[3].compare("SCS.ObservationInformation") )
    {
        sendObservationInfo( strtod(vstrTokens[1].c_str(), NULL)*1e6, vstrTokens[5].c_str(), vstrTokens[4].c_str() );
        return;
    }
// PJP
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


void cStationControllerKATCPClient::sendNoiseDiodeState(int64_t i64Timestamp_us, int32_t i32NoiseDiodeState, bool bNoiseDiodeSelect, const string &strStatus)
{
    // NoiseDiodeState is a bitfield which needs to be unpacked.
    // The description is in the Rx to SCS ICD.

    // For security. This should be only a 16-bit value.
    string strSource;
    switch (i32NoiseDiodeState & 0xFF)
    {
        case 1: strSource = "NONE";
                break;
        case 2: strSource = "DBBC";
                break;
        case 3: strSource = "ROACH";
                break;
        case 4: strSource = "SW";
                break;
        default: strSource = "UNKNOWN";
                break;
    }

    uint32_t ui32NoiseDiodeLevel       = (i32NoiseDiodeState & 0x0000FF00) >> 8;

    uint32_t i32NoiseDiodePWMMark      = (i32NoiseDiodeState & 0xFF000000) >> 24;

    double dNoiseDiodePWMFrequency;
    switch ((i32NoiseDiodeState & 0x00FF0000) >> 16)
    {
    case 1: dNoiseDiodePWMFrequency = 1.0;
            break;
    case 2: dNoiseDiodePWMFrequency = 10.0;
            break;
    case 3: dNoiseDiodePWMFrequency = 100.0;
            break;
    default: dNoiseDiodePWMFrequency = -1;
            break;
    }

    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    if (bNoiseDiodeSelect)
    {
        for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
        {
            cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
            pHandler->rNoiseDiode6_7GHzInputSource_callback(i64Timestamp_us, strSource, strStatus);
            pHandler->rNoiseDiode6_7GHzLevel_callback(i64Timestamp_us, ui32NoiseDiodeLevel, strStatus);
            pHandler->rNoiseDiode6_7GHzPWMMark_callback(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
            pHandler->rNoiseDiode6_7GHzPWMFrequency_callback(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
        }

        for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
        {
            boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
            pHandler->rNoiseDiode6_7GHzInputSource_callback(i64Timestamp_us, strSource, strStatus);
            pHandler->rNoiseDiode6_7GHzLevel_callback(i64Timestamp_us, ui32NoiseDiodeLevel, strStatus);
            pHandler->rNoiseDiode6_7GHzPWMMark_callback(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
            pHandler->rNoiseDiode6_7GHzPWMFrequency_callback(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
        }
    }
    else
    {
        for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
        {
            cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
            pHandler->rNoiseDiode5GHzInputSource_callback(i64Timestamp_us, strSource, strStatus);
            pHandler->rNoiseDiode5GHzLevel_callback(i64Timestamp_us, ui32NoiseDiodeLevel, strStatus);
            pHandler->rNoiseDiode5GHzPWMMark_callback(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
            pHandler->rNoiseDiode5GHzPWMFrequency_callback(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
        }

        for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
        {
            boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
            pHandler->rNoiseDiode5GHzInputSource_callback(i64Timestamp_us, strSource, strStatus);
            pHandler->rNoiseDiode5GHzLevel_callback(i64Timestamp_us, ui32NoiseDiodeLevel, strStatus);
            pHandler->rNoiseDiode5GHzPWMMark_callback(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
            pHandler->rNoiseDiode5GHzPWMFrequency_callback(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
        }
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

void cStationControllerKATCPClient::sendReceiverGain5GHzLcp(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverGain5GHzLcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverGain5GHzLcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverGain5GHzRcp(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverGain5GHzRcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverGain5GHzRcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverGain6_7GHzLcp(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverGain6_7GHzLcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverGain6_7GHzLcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }
}

void cStationControllerKATCPClient::sendReceiverGain6_7GHzRcp(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->receiverGain6_7GHzRcp_callback(i64Timestamp_us, dGain_dB, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->receiverGain6_7GHzRcp_callback(i64Timestamp_us, dGain_dB, strStatus);
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

void cStationControllerKATCPClient::sendAntennaBeamwidth(int64_t i64Timestamp_us, const string &strAntennaBeamwidth, const std::string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->antennaBeamwidth_callback(i64Timestamp_us, strAntennaBeamwidth, strStatus);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->antennaBeamwidth_callback(i64Timestamp_us, strAntennaBeamwidth, strStatus);
    }
}

void cStationControllerKATCPClient::sendObservationInfo(int64_t i64Timestamp_us, const string &strObservationInformation, const std::string &strStatus)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->observationInfo_callback(strObservationInformation);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->observationInfo_callback(strObservationInformation);
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
