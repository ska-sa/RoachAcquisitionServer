#ifndef ROACH_KATCP_CLIENT_H
#define ROACH_KATCP_CLIENT_H

//System includes


//Library includes

//Local includes
#include "AVNAppLibs/KATCP/KATCPClientBase.h"

//Implementation of KATCP client for fetching register values from the ROACH

class cRoachKATCPClient : public cKATCPClientBase
{
public:
    class cCallbackInterface : public cKATCPClientBase::cCallbackInterface
    {
    public:               
        //Roach values
        virtual void                                    accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NSamples) = 0;
        virtual void                                    narrowBandChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo) = 0;
        virtual void                                    frequencyFs_callback(double dFrequencyFs_MHz) = 0;
        virtual void                                    sizeOfFFTs_callback(uint32_t u32CoarseSize_nSamp, uint32_t u32FineSize_nSamp) = 0;
        virtual void                                    coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask) = 0;
        virtual void                                    adcAttenuation_callback(int64_t i64Timestamp_us, double dAttenuationChan0_dB, double dAttenuationChan1_dB) = 0;
    };
    
    cRoachKATCPClient(const std::string &strServerAddress, uint16_t u16Port = 7147);
    cRoachKATCPClient();
    ~cRoachKATCPClient();

private:
    void                                                processKATCPMessage(const std::vector<std::string> &vstrMessageTokens);
    
    //Roach values
    void                                                sendAccumulationLength(int64_t i64Timestamp_us, uint32_t u32NFrames);
    void                                                sendNarrowBandChannelSelect(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
    void                                                sendFrequencyFs(double dFrequencyFs_MHz);
    void                                                sendSizeOfFFTs(uint32_t u32CoarseSize_nSamp, uint32_t u32FineSize_nSamp);
    void                                                sendCoarseFFTShiftMask(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
    void                                                sendAdcAttenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB, double dAttenuationChan1_dB);
};

#endif // ROACH_KATCP_CLIENT_H
