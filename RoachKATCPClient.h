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
        //virtual void                                    stokesEnabled_callback(int64_t i64Timestamp_us, bool bEnabled) = 0;
        virtual void                                    accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NFrames) = 0;
        virtual void                                    coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo) = 0;
        virtual void                                    frequencyFs_callback(double dFrequencyFs_Hz) = 0;
        virtual void                                    sizeOfCoarseFFT_callback(uint32_t u32CoarseSize_nSamp) = 0;
        virtual void                                    sizeOfFineFFT_callback(uint32_t u32FineSize_nSamp) = 0;
        virtual void                                    coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask) = 0;
        virtual void                                    dspGain_callback(int64_t i64Timestamp_us, double dDspGain) = 0;
        virtual void                                    attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB) = 0;
        virtual void                                    attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB) = 0;
        virtual void                                    noiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoideDiodeEnabled)= 0;
        virtual void                                    noiseDiodeDutyCycleEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeDutyCyleEnabled) = 0;
        virtual void                                    noiseDiodeDutyCycleOnDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums) = 0;
        virtual void                                    noiseDiodeDutyCycleOffDuration_callback(int64_t i64Timestamp_us, uint32_t u32NAccums) = 0;
        virtual void                                    overflowsRegs_callback(int64_t i64Timestamp_us, uint32_t u32OverflowRegs) = 0;
        virtual void                                    eth10GbEUp_callback(int64_t i64Timestamp_us, bool bEth10GbEUp) = 0;
        virtual void                                    ppsCount_callback(int64_t i64Timestamp_us, uint32_t u32PPSCount) = 0;
        virtual void                                    clockFrequency_callback(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz) = 0;
    };

    cRoachKATCPClient(const std::string &strServerAddress, uint16_t u16Port = 7147);
    cRoachKATCPClient();
    ~cRoachKATCPClient();

    bool                                                writeRoachRegister(const std::string &strRegisterName, uint32_t u32Value);
    bool                                                readRoachRegister(const std::string &strRegisterName, uint32_t &u32Value);
    void                                                readAllRegisters(uint32_t u32SleepTime_ms = 0); //Reads all specified register values once

private:
    void                                                processKATCPMessage(const std::vector<std::string> &vstrMessageTokens);

    void                                                threadReadFunction();
    void                                                threadWriteFunction();

    //Roach values
    //void                                                sendStokesEnabled(int64_t i64Timestamp_us, bool bEnabled);
    void                                                sendAccumulationLength(int64_t i64Timestamp_us, uint32_t u32NFrames);
    void                                                sendCoarseChannelSelect(int64_t i64Timestamp_us, uint32_t u32ChannelNo);
    void                                                sendFrequencyFs(double dFrequencyFs_Hz);
    void                                                sendSizeOfCoarseFFT(uint32_t u32CoarseSize_nSamp);
    void                                                sendSizeOfFineFFT(uint32_t u32CoarseSize_nSamp);
    void                                                sendCoarseFFTShiftMask(int64_t i64Timestamp_us, uint32_t u32ShiftMask);
    void                                                sendDspGain(int64_t i64Timestamp_us, double dDspGain);
    void                                                sendADCChan0Attenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB);
    void                                                sendADCChan1Attenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB);
    void                                                sendNoiseDiodeEnabled(int64_t i64Timestamp_us, bool bNoiseDiodeEnabled);
    void                                                sendNoiseDiodeDutyCycleEnabled(int64_t i64Timestamp_us, bool bNoiseDiodeDutyCyleEnabled);
    void                                                sendNoiseDiodeDutyCycleOnDuration(int64_t i64Timestamp_us, uint32_t u32NAccums);
    void                                                sendNoiseDiodeDutyCycleOffDuration(int64_t i64Timestamp_us, uint32_t u32NAccums);
    void                                                sendOverflowsRegs(int64_t i64Timestamp_us, uint32_t u32OverflowRegs);
    void                                                sendEth10GbEUp(int64_t i64Timestamp_us, bool bEth10GbEUp);
    void                                                sendPPSCount(int64_t i64Timestamp_us, uint32_t u32PPSCount);
    void                                                sendClockFrequency(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz);

};

#endif // ROACH_KATCP_CLIENT_H
