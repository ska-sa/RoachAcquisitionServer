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
        virtual void                                    accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames) = 0;
        virtual void                                    coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo) = 0;
        virtual void                                    frequencyFs_callback(double dFrequencyFs_MHz) = 0;
        virtual void                                    sizeOfCoarseFFT_callback(uint32_t u32CoarseSize_nSamp) = 0;
        virtual void                                    sizeOfFineFFT_callback(uint32_t u32FineSize_nSamp) = 0;
        virtual void                                    coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask) = 0;
        virtual void                                    attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB) = 0;
        virtual void                                    attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB) = 0;
    };

    cRoachKATCPClient(const std::string &strServerAddress, uint16_t u16Port = 7147);
    cRoachKATCPClient();
    ~cRoachKATCPClient();

    bool                                                readRoachRegister(const std::string &strRegisterName, uint32_t &u32Value);

private:
    void                                                processKATCPMessage(const std::vector<std::string> &vstrMessageTokens);

    void                                                threadReadFunction();
    void                                                threadWriteFunction();


    //Roach values
    void                                                sendAccumulationLength(int64_t i64Timestamp_us, uint32_t u32NFrames);
    void                                                sendCoarseChannelSelect(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
    void                                                sendFrequencyFs(double dFrequencyFs_MHz);
    void                                                sendSizeOfCoarseFFT(uint32_t u32CoarseSize_nSamp);
    void                                                sendSizeOfFineFFT(uint32_t u32CoarseSize_nSamp);
    void                                                sendCoarseFFTShiftMask(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
    void                                                sendADCChan0Attenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB);
    void                                                sendADCChan1Attenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB);
};

#endif // ROACH_KATCP_CLIENT_H
