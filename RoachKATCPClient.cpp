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

    m_pSocket->write(oSS.str());

    //Read a line of KATCP response (until  "\n" is encountered)
    //This returns a vector of space-delimited tokens
    do
    {
        vstrMessageTokens = readNextKATCPMessage(1000);

        //        //Debug:
        //        cout << "cRoachKATCPClient::readRoachRegister(): Got KATCP line: ";
        //        for(uint32_t ui = 0; ui < vstrMessageTokens.size(); ui++)
        //        {
        //            cout << vstrMessageTokens[ui] << " ";
        //        }
        //        cout << endl;
    }

    while(vstrMessageTokens[0].compare("!wordread")); //Read until the reponse

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

void cRoachKATCPClient::threadReadFunction()
{
    //Not used here. Reading and writing is done synchrously
    //The write thread function is overloaded to do this.

    cout << "cRoachKATCPClient::threadReadFunction(): Entering thread read function." << endl;
    cout << "cRoachKATCPClient::threadReadFunction(): Leaving thread read function." << endl;
}

void cRoachKATCPClient::threadWriteFunction()
{
    cout << "cRoachKATCPClient::threadWriteFunction(): Entering thread write function." << endl;

    vector<string> vstrMessageTokens;

    uint32_t u32Value = 0;
    double dADCAttenuation_dB = 0.0;
    uint32_t u32SleepTime_ms = 50;

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
        if( readRoachRegister(string("accumulation_length"), u32Value) )
        {
            sendAccumulationLength(AVN::getTimeNow_us(), u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote accumulation_length" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: accumulation_length" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if( readRoachRegister(string("coarse_channel_select"), u32Value) )
        {
            sendCoarseChannelSelect(AVN::getTimeNow_us(), u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote coarse_channel_select" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: coarse_channel_select" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if( readRoachRegister(string("sampling_frequency_mhz"), u32Value) )

        {
            sendFrequencyFs((double)u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote sampling_frequency_mhz" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: sampling_frequency_mhz" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if( readRoachRegister(string("coarse_fft_size_nsamp"), u32Value))
        {
            sendSizeOfCoarseFFT(u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote coarse_fft_size_nsamp" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: coarse_fft_size" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if(readRoachRegister(string("fine_fft_size_nsamp"), u32Value))
        {
            sendSizeOfFineFFT(u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote fine_fft_size_nsamp" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: fine_fft_size" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if( readRoachRegister(string("coarse_fft_shift_mask"), u32Value) )
        {
            sendCoarseFFTShiftMask(AVN::getTimeNow_us(), u32Value);
            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote coarse_fft_shift_mask" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: coarse_fft_shift_mask" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if( readRoachRegister(string("adc0_attenuation_db"), u32Value) )
        {
            //KATADC 0 dB to 31.5 dB in 0.5 dB steps (https://casper.berkeley.edu/wiki/KatADC)
            dADCAttenuation_dB = u32Value * 0.5;
            sendADCChan0Attenuation(AVN::getTimeNow_us(), dADCAttenuation_dB);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote adc0_attentuation = " << dADCAttenuation_dB << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: adc0_attenuation_db" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));

        if( readRoachRegister(string("adc1_attenuation_db"), u32Value) )
        {
            //KATADC 0 dB to 31.5 dB in 0.5 dB steps (https://casper.berkeley.edu/wiki/KatADC)
            dADCAttenuation_dB = u32Value * 0.5;
            sendADCChan1Attenuation(AVN::getTimeNow_us(), dADCAttenuation_dB);

            //cout << "cRoachKATCPClient::threadWriteFunction(): Wrote adc1_attentuation" << endl;
        }
        else
        {
            cout << "cRoachKATCPClient::threadWriteFunction(): Failed to read register: adc1_attenuation_db" << endl;
        }

        //Wait a bit before next read
        boost::this_thread::sleep_for(boost::chrono::milliseconds(u32SleepTime_ms));
    }

    cout << "cRoachKATCPClient::threadWriteFunction(): Leaving thread write function." << endl;
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
        //pHandler->adcAttenuation_callback(0, 0.0, 0.0);
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        boost::shared_ptr<cCallbackInterface> pHandler = boost::dynamic_pointer_cast<cCallbackInterface>(m_vpCallbackHandlers_shared[ui]);
        pHandler->attenuationADCChan1_callback(i64Timestamp_us, dAttenuationChan1_dB);
    }
}
