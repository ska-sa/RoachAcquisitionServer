//System includes

//Library includes

//Local includes
#include "RoachKATCPClient.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"

using namespace std;

cRoachKATCPClient::cRoachKATCPClient() :
    cKATCPClientBase()
{
}

cRoachKATCPClient::~cRoachKATCPClient()
{
}

void cRoachKATCPClient::processKATCPMessage(const std::vector<std::string> &vstrMessageTokens)
{
    //Note used in this implementation
}

bool cRoachKATCPClient::readRoachRegister(const string &strRegisterName, uint32_t &u32Value)
{
    //Reads a uint32_t value from the specified Roach FPGA register.
    //Returns false on read failure or true on success

    vector<string> vstrMessageTokens;

    //Contruct send command (?wordread <register name> <offset> <length>)
    stringstream oSS;
    oSS << "?wordread ";
    oSS << strRegisterName;
    oSS << " 0 1\n"; //offset zero, word length 1. NB end of line.

    m_pSocket->write(oSS.str(), 1000);

    //Read a line of KATCP response (until  "\n" is encountered)
    //This returns a vector of space-delimited tokens
    while(1)
    {
        vstrMessageTokens = readNextKATCPMessage(1000);

        //        //Debug:
        //        cout << "cRoachKATCPClient::readRoachRegister(): Got KATCP line: ";
        //        for(uint32_t ui = 0; ui < vstrMessageTokens.size(); ui++)
        //        {
        //            cout << vstrMessageTokens[ui] << " ";
        //        }
        //        cout << endl;

        if(disconnectRequested())
            return false;

        if(!vstrMessageTokens.size())
            continue;

        //Read until the reponse
        if(!vstrMessageTokens[0].compare("!wordread"))
            break;
    }

    //Return message is of format: "!wordread ok 0x0000" where 0x0000 is the corresponding hex value
    //Check that the first 2 tokens are correct and then convert the 3rd
    if(vstrMessageTokens.size() < 3)
    {
        return false;
    }

    if(!vstrMessageTokens[0].compare("!wordread") && !vstrMessageTokens[1].compare("ok"))
    {
        u32Value = strtoul(vstrMessageTokens[2].c_str(), NULL, 16);
        return true;
    }
    else
    {
        return false;
    }
}

bool cRoachKATCPClient::writeRoachRegister(const std::string &strRegisterName, uint32_t u32Value)
{
    vector<string> vstrMessageTokens;

    //Contruct send command (?wordwrite <register name> <offset> <length>)
    stringstream oSS;
    oSS << "?wordwrite ";
    oSS << strRegisterName;
    oSS << " 0 "; //offset zero, word length 1.
    oSS << u32Value;
    oSS << "\n"; //NB end of line.

    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if(!m_pSocket->write(oSS.str(), 1000))
        {
            return false;
        }

        //Read the response
        while(1)
        {
            vstrMessageTokens = readNextKATCPMessage(1000);

                    //Debug:
                    cout << "cRoachKATCPClient::readRoachRegister(): Got KATCP line: ";
                    for(uint32_t ui = 0; ui < vstrMessageTokens.size(); ui++)
                    {
                        cout << vstrMessageTokens[ui] << " ";
                    }
                    cout << endl;

            if(disconnectRequested())
                return false;

            if(!vstrMessageTokens.size())
                continue;

            //Read until the reponse
            if(!vstrMessageTokens[0].compare("!wordwrite"))
            {
                if(!vstrMessageTokens[1].compare("ok"))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
}

void cRoachKATCPClient::threadReadFunction()
{
    // ROACH KATCP communication is synchronous, so it's all handled in the read thread.
    // In contrast to the station control one which may arbitrarily send subscribed values.

    cout << "cRoachKATCPClient::threadReadFunction(): Entering thread read function." << endl;

    vector<string> vstrMessageTokens;

    //Read KATCP header info that is sent on connect
    while(!disconnectRequested())
    {
        vstrMessageTokens = readNextKATCPMessage(1000);

        //        //Debug:
        //        //Print all the tokens
        //        cout << ""cRoachKATCPClient::threadWriteFunction(): KATCP Header: ";
        //        for(uint32_t ui = 0; ui < vstrMessageTokens.size(); ui++)
        //        {
        //            cout << vstrMessageTokens[ui] << " ";
        //        }
        //        cout << endl;

        if(!vstrMessageTokens.size())
        {
            //cout << "cRoachKATCPClient::threadWriteFunction(): Got all header text moving on to register monitoring." << endl;
            break;
        }
    }

    while(!disconnectRequested())
    {
      //Read all registers once and execute relevant callbacks
      readAllRegisters(10);
    }

    cout << "cRoachKATCPClient::threadReadFunction(): Leaving thread read function." << endl;
}

void cRoachKATCPClient::threadWriteFunction()
{
    //Not used

    cout << "cRoachKATCPClient::threadWriteFunction(): Entering thread write function. (Unused)" << endl;
    cout << "cRoachKATCPClient::threadWriteFunction(): Leaving thread write function. (Unused)" << endl;
}

void cRoachKATCPClient::readAllRegisters(uint32_t u32SleepTime_ms)
{
    //Attempts Reads each Roach spectrometer register of interest once and executes the relevant callback
    //The KATCP mutex is locked before each read to allow safe multiplexing with write operations.

    uint32_t u32Value = 0;
    double dADCAttenuation_dB = 0.0;

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("stokes_enable"), u32Value) )
        {
            sendStokesEnabled(AVN::getTimeNow_us(), (bool)(u32Value & 0x00000001));
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote stokes_enable" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: stokes_enable" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("accumulation_length"), u32Value) )
        {
            sendAccumulationLength(AVN::getTimeNow_us(), u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote accumulation_length: " << m_i32AccumulationLength_nVal << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: accumulation_length" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("coarse_channel_select"), u32Value) )
        {
            sendCoarseChannelSelect(AVN::getTimeNow_us(), u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote coarse_channel_select" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: coarse_channel_select" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("sampling_frequency_mhz"), u32Value) )

        {
            sendFrequencyFs((double)u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote sampling_frequency_mhz" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: sampling_frequency_mhz" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("coarse_fft_size_nsamp"), u32Value))
        {
            sendSizeOfCoarseFFT(u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote coarse_fft_size_nsamp" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: coarse_fft_size" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if(readRoachRegister(string("fine_fft_size_nsamp"), u32Value))
        {
            sendSizeOfFineFFT(u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote fine_fft_size_nsamp" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: fine_fft_size" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("coarse_fft_shift_mask"), u32Value) )
        {
            sendCoarseFFTShiftMask(AVN::getTimeNow_us(), u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote coarse_fft_shift_mask" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: coarse_fft_shift_mask" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("adc0_atten"), u32Value) )
        {
            //KATADC 0 dB to 31.5 dB in 0.5 dB steps (https://casper.berkeley.edu/wiki/KatADC)
            dADCAttenuation_dB = u32Value * 0.5;
            sendADCChan0Attenuation(AVN::getTimeNow_us(), dADCAttenuation_dB);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote adc0_atten = " << dADCAttenuation_dB << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: adc0_atten" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("adc1_atten"), u32Value) )
        {
            //KATADC 0 dB to 31.5 dB in 0.5 dB steps (https://casper.berkeley.edu/wiki/KatADC)
            dADCAttenuation_dB = u32Value * 0.5;
            sendADCChan1Attenuation(AVN::getTimeNow_us(), dADCAttenuation_dB);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote adc1_atten" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: adc1_atten" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("noise_diode_en"), u32Value) )
        {
            sendNoiseDiodeEnabled(AVN::getTimeNow_us(), (bool)(u32Value & 0x00000001));

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote noise_diode_en" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: noise_diode_en" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("noise_diode_duty_cycle_en"), u32Value) )
        {
            sendNoiseDiodeDutyCycleEnabled(AVN::getTimeNow_us(), (bool)(u32Value & 0x00000001));

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote noise_diode_duty_cycle_en" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: noise_diode_duty_cycle_en" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("noise_diode_on_length"), u32Value) )
        {
            sendNoiseDiodeDutyCycleOnDuration(AVN::getTimeNow_us(), u32Value);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote noise_diode_on_length " << u32Value << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: noise_diode_on_length" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("noise_diode_off_length"), u32Value) )
        {
            sendNoiseDiodeDutyCycleOffDuration(AVN::getTimeNow_us(), u32Value);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote noise_diode_off_length" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: noise_diode_off_length" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("overflows"), u32Value) )
        {
            sendOverflowsRegs(AVN::getTimeNow_us(), u32Value);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote overflows" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: overflows" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("tgbe0_linkup"), u32Value) )
        {
            sendEth10GbEUp(AVN::getTimeNow_us(), u32Value);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote tgbe0_linkup" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: tgbe0_linkup" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("pps_cnt"), u32Value) )
        {
            sendPPSCount(AVN::getTimeNow_us(), u32Value);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote pps_cnt: " << u32Value << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: pps_cnt " << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

    if(disconnectRequested())
        return;
    {
        boost::unique_lock<boost::mutex> oLock(m_oKATCPMutex);
        if( readRoachRegister(string("clk_frequency"), u32Value) )
        {
            sendClockFrequency(AVN::getTimeNow_us(), u32Value);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote clk_frequency" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: clk_frequency" << endl;
        }
    }

    //Wait a bit before next read
    boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));
}

void cRoachKATCPClient::sendStokesEnabled(int64_t i64Timestamp_us, bool bEnabled)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->stokesEnabled_callback(i64Timestamp_us, bEnabled);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->stokesEnabled_callback(i64Timestamp_us, bEnabled);
    }
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

void cRoachKATCPClient::sendCoarseChannelSelect(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->coarseChannelSelect_callback(i64Timestamp_us, u32ChannelNo);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->coarseChannelSelect_callback(i64Timestamp_us, u32ChannelNo);
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

void cRoachKATCPClient::sendSizeOfCoarseFFT(uint32_t u32CoarseSize_nSamp)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->sizeOfCoarseFFT_callback(u32CoarseSize_nSamp);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->sizeOfCoarseFFT_callback(u32CoarseSize_nSamp);
    }
}

void cRoachKATCPClient::sendSizeOfFineFFT(uint32_t u32FineSize_nSamp)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->sizeOfFineFFT_callback(u32FineSize_nSamp);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->sizeOfFineFFT_callback(u32FineSize_nSamp);
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

void cRoachKATCPClient::sendADCChan0Attenuation(int64_t i64Timestamp_us, double dAttenuationChan0_dB)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->attenuationADCChan0_callback(i64Timestamp_us, dAttenuationChan0_dB);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->attenuationADCChan0_callback(i64Timestamp_us, dAttenuationChan0_dB);
    }
}

void cRoachKATCPClient::sendADCChan1Attenuation(int64_t i64Timestamp_us, double dAttenuationChan1_dB)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->attenuationADCChan1_callback(i64Timestamp_us, dAttenuationChan1_dB);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->attenuationADCChan1_callback(i64Timestamp_us, dAttenuationChan1_dB);
    }
}

void cRoachKATCPClient::sendNoiseDiodeEnabled(int64_t i64Timestamp_us, bool bNoiseDiodeEnabled)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeEnabled_callback(i64Timestamp_us, bNoiseDiodeEnabled);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeEnabled_callback(i64Timestamp_us, bNoiseDiodeEnabled);
    }
}

void cRoachKATCPClient::sendNoiseDiodeDutyCycleEnabled(int64_t i64Timestamp_us, bool bNoiseDiodeDutyCyleEnabled)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeDutyCycleEnabled_callback(i64Timestamp_us, bNoiseDiodeDutyCyleEnabled);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeDutyCycleEnabled_callback(i64Timestamp_us, bNoiseDiodeDutyCyleEnabled);
    }
}

void cRoachKATCPClient::sendNoiseDiodeDutyCycleOnDuration(int64_t i64Timestamp_us, uint32_t u32NAccums)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeDutyCycleOnDuration_callback(i64Timestamp_us, u32NAccums);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeDutyCycleOnDuration_callback(i64Timestamp_us, u32NAccums);
    }
}

void cRoachKATCPClient::sendNoiseDiodeDutyCycleOffDuration(int64_t i64Timestamp_us, uint32_t u32NAccums)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->noiseDiodeDutyCycleOffDuration_callback(i64Timestamp_us, u32NAccums);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->noiseDiodeDutyCycleOffDuration_callback(i64Timestamp_us, u32NAccums);
    }
}

void cRoachKATCPClient::sendOverflowsRegs(int64_t i64Timestamp_us, uint32_t u32OverflowRegs)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->overflowsRegs_callback(i64Timestamp_us, u32OverflowRegs);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->overflowsRegs_callback(i64Timestamp_us, u32OverflowRegs);
    }
}

void cRoachKATCPClient::sendEth10GbEUp(int64_t i64Timestamp_us, bool bEth10GbEUp)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->eth10GbEUp_callback(i64Timestamp_us, bEth10GbEUp);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->eth10GbEUp_callback(i64Timestamp_us, bEth10GbEUp);
    }
}

void cRoachKATCPClient::sendPPSCount(int64_t i64Timestamp_us, uint32_t u32PPSCount)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->ppsCount_callback(i64Timestamp_us, u32PPSCount);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->ppsCount_callback(i64Timestamp_us, u32PPSCount);
    }
}

void cRoachKATCPClient::sendClockFrequency(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz)
{
    boost::shared_lock<boost::shared_mutex> oLock;

    //Note the vector contains the base type callback handler pointer so cast to the derived version is this class
    //to call function added in the derived version of the callback handler interface class

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        cCallbackInterface *pHandler = dynamic_cast<cCallbackInterface*>(m_vpCallbackHandlers[ui]);
        pHandler->clockFrequency_callback(i64Timestamp_us, u32ClockFrequency_Hz);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->clockFrequency_callback(i64Timestamp_us, u32ClockFrequency_Hz);
    }
}
