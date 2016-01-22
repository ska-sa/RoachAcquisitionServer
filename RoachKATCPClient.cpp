//System includes

//Library includes

//Local includes
#include "RoachKATCPClient.h"

using namespace std;

cRoachKATCPClient::cRoachKATCPClient(const string &strServerAddress, uint16_t u16Port) :
    cKATCPClientBase(strServerAddress, u16Port)
{
}

cRoachKATCPClient::cRoachKATCPClient() :
    cKATCPClientBase()
{
}

cRoachKATCPClient::~cRoachKATCPClient()
{
}

void cRoachKATCPClient::sendAccumulationLength(int64_t i64Timestamp_us, uint32_t u32NFrames)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->accumulationLength_callback(i64Timestamp_us, u32NFrames);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->accumulationLength_callback(i64Timestamp_us, u32NFrames);
    }
}

void cRoachKATCPClient::sendNarrowBandChannelSelect(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->narrowBandChannelSelect_callback(i64Timestamp_us, u32ChannelNo);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->narrowBandChannelSelect_callback(i64Timestamp_us, u32ChannelNo);
    }
}

void cRoachKATCPClient::sendFrequencyFs(double dFrequencyFs_MHz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->frequencyFs_callback(dFrequencyFs_MHz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->frequencyFs_callback(dFrequencyFs_MHz);
    }
}

void cRoachKATCPClient::sendSizeOfFFTs(uint32_t u32CoarseSize_nSamp, uint32_t u32FineSize_nSamp)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->sizeOfFFTs_callback(u32CoarseSize_nSamp, u32FineSize_nSamp);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->sizeOfFFTs_callback(u32CoarseSize_nSamp, u32FineSize_nSamp);
    }
}

void cRoachKATCPClient::sendCoarseFFTShiftMask(int64_t i64Timestamp_us, uint32_t u32ShiftMask)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->coarseFFTShiftMask_callback(i64Timestamp_us, u32ShiftMask);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->coarseFFTShiftMask_callback(i64Timestamp_us, u32ShiftMask);
    }
}

void cRoachKATCPClient::sendAdcAttenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB, double dAttenuationChan1_dB)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->adcAttenuation_callback(i64Timestamp_us, dAttenuationChan0_dB, dAttenuationChan1_dB);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->adcAttenuation_callback(i64Timestamp_us, dAttenuationChan0_dB, dAttenuationChan1_dB);
    }
}

void cRoachKATCPClient::processKATCPMessage(const vector<string> &vstrTokens)
{
    try
    {
        if(!vstrTokens[0].compare("#accumulationLength"))
        {
            sendAccumulationLength( strtoll(vstrTokens[1].c_str(), NULL, 10), strtoul(vstrTokens[2].c_str(), NULL, 10) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#narrowBandChannelSelect"))
        {
            sendNarrowBandChannelSelect( strtoll(vstrTokens[1].c_str(), NULL, 10), strtoul(vstrTokens[2].c_str(), NULL, 10) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#frequencyFs"))
        {
            sendFrequencyFs( strtod(vstrTokens[1].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#sizeOfFFTs"))
        {
            sendSizeOfFFTs( strtol(vstrTokens[1].c_str(), NULL, 10), strtol(vstrTokens[2].c_str(), NULL, 10) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#coarseFFTShiftMask"))
        {
            sendCoarseFFTShiftMask( strtoll(vstrTokens[1].c_str(), NULL, 10), strtoul(vstrTokens[2].c_str(), NULL, 10) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }

    try
    {
        if(!vstrTokens[0].compare("#adcAttenuation"))
        {
            sendAdcAttenuation( strtoll(vstrTokens[1].c_str(), NULL, 10), strtod(vstrTokens[2].c_str(), NULL), strtod(vstrTokens[2].c_str(), NULL) );
            return;
        }
    }
    catch(out_of_range &oError)
    {
    }
}
