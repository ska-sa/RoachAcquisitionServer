
//System includes
#include <sstream>
#include <climits>

//Library include
#include <boost/make_shared.hpp>

//Local includes
#include "HDF5FileWriter.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"
#include "AVNDataTypes/SpectrometerDataStream/SpectrometerDefinitions.h"

using namespace std;

cHDF5FileWriter::cHDF5FileWriter(const string &strRecordingDirectory) :
    m_bStreamChanged(false),
    m_strRecordingDirectory(strRecordingDirectory),
    m_strFilename(string("Undefined")),
    m_i64LastPrintTime_us(0),
    m_eState(IDLE)
{
    m_oDataStreamInterpreter.registerCallbackHandler(this); //Have data stream handler push interpreted packet data back to this class.
}

cHDF5FileWriter::~cHDF5FileWriter()
{
    stopRecording();

    m_oDataStreamInterpreter.setIsRunning(false);

    cout << "cHDF5FileWriter::~cHDF5FileWriter() exiting." << endl;
}

void cHDF5FileWriter::startRecording(const string &strFilenamePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    if(getState() != IDLE)
    {
        cout << "Warning HDF5 recording is currently active ignoring." << endl;
        return;
    }

    cout << "cHDF5FileWriter::startRecording(): Got request to start file recording at " << AVN::stringFromTimestamp_full(i64StartTime_us) << endl;

    m_strFilenamePrefix = strFilenamePrefix;
    m_i64RequestedStartTime_us = i64StartTime_us;
    m_i64Duration_us = i64Duration_us;

    setState(REQUEST_START);
}

void cHDF5FileWriter::stopRecording()
{
    if(getState() == IDLE)
        return;

    setState(REQUEST_STOP);
}

void cHDF5FileWriter::setState(state eState)
{
    boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
    m_eState = eState;

    //For when recording is starting
    if(m_eState == REQUEST_START)
    {
        m_strFilename = string("");
        m_i64ActualStartTime_us = 0;

        notifyAllRecordingStarted();
    }
}

cHDF5FileWriter::state cHDF5FileWriter::getState()
{
    boost::shared_lock<boost::shared_mutex>  oLock(m_oMutex);
    return m_eState;
}

void cHDF5FileWriter::offloadData_callback(char* cpData, uint32_t u32Size_B)
{
    //Pass data on directly to the stream interpreter
    m_oDataStreamInterpreter.offloadData_callback(cpData, u32Size_B);
}

void cHDF5FileWriter::getNextFrame_callback(const std::vector<int> &vi32Chan0, const std::vector<int> &vi32Chan1, const std::vector<int> &vi32Chan2, std::vector<int> &vi32Chan3,
                                            const cSpectrometerHeader &oHeader)
{
    //Update the frame rate and the frame size every frame and check for changes

    if( abs( m_i64FrameInterval_us - (oHeader.getTimestamp_us() - m_i64LastTimestamp_us) ) > 1 )
    {
        //Allow difference of 1 due to rounding errors in microsec timestamp resolution.
        //This will also trigger a new file if any sort of stream discontinuity occurs.

        cout << "getNextFrame_callback(): Detected stream change in frame update interval. Was " << m_i64FrameInterval_us << " got " <<  oHeader.getTimestamp_us() - m_i64LastTimestamp_us << endl;

        m_i64FrameInterval_us = oHeader.getTimestamp_us() - m_i64LastTimestamp_us;
        m_bStreamChanged = true;
    }
    else
    {
        m_bStreamChanged = false;
    }


    if(m_u32FrameSize_nVal != vi32Chan0.size())
    {
        cout << "getNextFrame_callback(): Detected stream change in frame size. Was " << m_u32FrameSize_nVal << " got " << vi32Chan0.size() << endl;

        m_u32FrameSize_nVal = vi32Chan0.size();
        m_bStreamChanged = true;
    }

    if(m_eLastDigitiserType != (AVN::Spectrometer::digitiserType)oHeader.getDigitiserType())
    {
        cout << "getNextFrame_callback(): Detected stream change in digitiser. Was \"" << AVN::Spectrometer::digitiserTypeToString(m_eLastDigitiserType)
             << "\" got \"" << AVN::Spectrometer::digitiserTypeToString(oHeader.getDigitiserType()) << "\"" << endl;

        m_eLastDigitiserType = (AVN::Spectrometer::digitiserType)oHeader.getDigitiserType();
        m_bStreamChanged = true;
    }

    m_i64LastTimestamp_us = oHeader.getTimestamp_us();

    //Then processing based of state
    switch(getState())
    {
    case IDLE:
        //Do nothing
        break;

    case REQUEST_START:
    {
        //Note notification callbacks for entering this state are triggered from the set state function.

        if(m_i64RequestedStartTime_us > m_i64LastTimestamp_us)
        {
            if(m_i64LastTimestamp_us - m_i64LastPrintTime_us > 500000) //Limit this output to ever 500 ms
            {
                cout << "Waiting to record..."
                     << " | Time now : " << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us)
                     << " | Start time : " << AVN::stringFromTimestamp_full(m_i64RequestedStartTime_us)
                     << " | Time to start : " << AVN::stringFromTimeDuration(m_i64RequestedStartTime_us - m_i64LastTimestamp_us)
                     << "\r" << std::flush;

                m_i64LastPrintTime_us = m_i64LastTimestamp_us;
                setRecordedDuration_us(m_i64LastTimestamp_us - m_i64RequestedStartTime_us);
            }

            //We haven't reach the request start time yet ignore this data...
            break;
        }

        cout << endl;

        //Otherwise make a new HDF5 file
        m_i64ActualStartTime_us = m_i64LastTimestamp_us;
        m_strFilename = makeFilename(m_strRecordingDirectory, m_strFilenamePrefix, m_i64ActualStartTime_us);

        m_pHDF5File = boost::make_shared<cSpectrometerHDF5OutputFile>(m_strFilename, m_eLastDigitiserType, m_u32FrameSize_nVal);

        if(m_i64Duration_us)
        {
            m_i64StopTime_us = m_i64ActualStartTime_us + m_i64Duration_us;
        }
        else
        {
            m_i64StopTime_us = LLONG_MAX; //0 duration implies record indefinitely.
        }

        setState(RECORDING);

        m_bStreamChanged = false; //Might be the first frame after a change

        cout << "cHDF5FileWriter::getNextFrame_callback(): Starting recording for file " << m_strFilename << endl;

        cout << endl;
        cout << "Recording:" << endl;
        cout << "--------- " << endl;
        cout << "Start time: " << AVN::stringFromTimestamp_full(m_i64ActualStartTime_us) << endl;
        if(m_i64StopTime_us == LLONG_MAX)
        {
            cout << "Stop time: On user request." << endl;
        }
        else
        {
            cout << "Stop time: " << AVN::stringFromTimestamp_full(m_i64StopTime_us) << endl;
        }

        //Note, no break here as we want to save this frame anyway, so execute the RECORDING STATE block as well
    }

    case RECORDING:
    {
        //Check if parameters have changed if so break and start a new file.
        if(m_bStreamChanged)
        {
            cout << "cHDF5FileWriter::getNextFrame_callback(): Dectect stream change closing file " << m_strFilename << endl;
            m_pHDF5File.reset(); //Closes current HDF5 file.
            setState(REQUEST_START);
            break;

            //The current (first different) frame will be lost. At present this seems like acceptable behavior.
            //If not this code will need to jump back to the previous state instead of exiting the function. This may
            //require more flags and a loop or doing this check above the switch.
        }

        //Otherwise write the frame to file...
        m_pHDF5File->addFrame(vi32Chan0, vi32Chan1, vi32Chan2, vi32Chan3, oHeader);

        if(m_i64LastTimestamp_us - m_i64LastPrintTime_us > 500000) //Limit this output to ever 500 ms
        {
            if(m_i64StopTime_us == LLONG_MAX)
            {
                cout << "Last timestamp : " << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us)
                     << " | Elapsed time: " << AVN::stringFromTimeDuration(m_i64LastTimestamp_us - m_i64ActualStartTime_us)
                     << "\r" << std::flush;
            }
            else
            {
                cout << "Last timestamp : " << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us)
                     << " | Elapsed time: " << AVN::stringFromTimeDuration(m_i64LastTimestamp_us - m_i64ActualStartTime_us)
                     << " | Time to stop : " << AVN::stringFromTimeDuration(m_i64StopTime_us - m_i64LastTimestamp_us)
                     << "\r" << std::flush;
            }

            m_i64LastPrintTime_us = m_i64LastTimestamp_us;
        }

        //Keep track of the duration (use thread safe function to alter variable as it can also be read from elsewhere)
        setRecordedDuration_us(m_i64LastTimestamp_us - m_i64ActualStartTime_us);

        //Check the duration of recording. Stop if necessary.
        if(m_i64LastTimestamp_us >= m_i64StopTime_us)
        {
            cout << endl << endl;
            setState(REQUEST_STOP);
        }

        break;
    }

    case REQUEST_STOP:
    {
        cout << "cHDF5FileWriter::getNextFrame_callback(): Reached end of specified recording duration or got stop request." << endl;
        cout << "cHDF5FileWriter::getNextFrame_callback(): Requesting closing of file." << endl;

        m_pHDF5File.reset(); //Closes current HDF5 file.

        cout << "cHDF5FileWriter::getNextFrame_callback(): Closed file " << m_strFilename << endl;

        setState(IDLE);

        notifyAllRecordingStopped();

        break;
    }


    }
}

std::string cHDF5FileWriter::makeFilename(const std::string &strDirectory, const std::string &strPrefix, int64_t i64Timestamp_us)
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    stringstream oSS;
    oSS << strDirectory;
#ifdef _WIN32
    if(strDirectory[strDirectory.length() - 1] != '\\')
        oSS << "\\";
#else
    if(strDirectory[strDirectory.length() - 1] != '/')
        oSS << "/";
#endif
    oSS << m_strFilenamePrefix;
    oSS << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us); //Note we use the actual packet timestamp and not the user requested start time.
    oSS << string(".h5");

    return oSS.str();
}

void  cHDF5FileWriter::setRecordingDirectory(const std::string &strRecordingDirectory)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);

    m_strRecordingDirectory = strRecordingDirectory;
}

void cHDF5FileWriter::setRecordedDuration_us(int64_t i64Duration_us)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);

    m_i64RecordedDuration_us = i64Duration_us;
}

int64_t cHDF5FileWriter::getRecordingStartTime_us()
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    return m_i64ActualStartTime_us;
}

int64_t cHDF5FileWriter::getRecordedDuration_us()
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    return m_i64RecordedDuration_us;
}

int64_t  cHDF5FileWriter::getRecordingStopTime_us()
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    return m_i64StopTime_us;
}

int64_t  cHDF5FileWriter::getRecordingTimeLeft_us()
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    return m_i64StopTime_us - m_i64LastTimestamp_us;
}

std::string cHDF5FileWriter::getFilename()
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    return m_strFilename;
}

bool cHDF5FileWriter::isRecordingEnabled()
{
    if(getState() == IDLE)
        return false;
    else
        return true;

}

void cHDF5FileWriter::waitForFileClosed()
{
    if(getState() != IDLE)
    {
        cout << "cHDF5FileWriter::waitForFileClose() Waiting for HDF5 file to close" << endl;
    }
    while(getState() != IDLE)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    cout << "cHDF5FileWriter::waitForFileClose() HDF5 file is closed." << endl;
}

void cHDF5FileWriter::registerCallbackHandler(cCallbackInterface *pNewHandler)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    m_vpCallbackHandlers.push_back(pNewHandler);

    cout << "cHDF5FileWriter::registerCallbackHandler(): Successfully registered callback handler: " << pNewHandler << endl;
}

void cHDF5FileWriter::registerCallbackHandler(boost::shared_ptr<cCallbackInterface> pNewHandler)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    m_vpCallbackHandlers_shared.push_back(pNewHandler);

    cout << "cHDF5FileWriter::registerCallbackHandler(): Successfully registered callback handler: " << pNewHandler.get() << endl;
}

void cHDF5FileWriter::deregisterCallbackHandler(cCallbackInterface *pHandler)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);
    bool bSuccess = false;

    //Search for matching pointer values and erase
    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size();)
    {
        if(m_vpCallbackHandlers[ui] == pHandler)
        {
            m_vpCallbackHandlers.erase(m_vpCallbackHandlers.begin() + ui);

            cout << "cHDF5FileWriter::deregisterCallbackHandler(): Deregistered callback handler: " << pHandler << endl;
            bSuccess = true;
        }
        else
        {
            ui++;
        }
    }

    if(!bSuccess)
    {
        cout << "cHDF5FileWriter::::deregisterCallbackHandler(): Warning: Deregistering callback handler: " << pHandler << " failed. Object instance not found." << endl;
    }
}

void cHDF5FileWriter::deregisterCallbackHandler(boost::shared_ptr<cCallbackInterface> pHandler)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);
    bool bSuccess = false;

    //Search for matching pointer values and erase
    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size();)
    {
        if(m_vpCallbackHandlers_shared[ui].get() == pHandler.get())
        {
            m_vpCallbackHandlers_shared.erase(m_vpCallbackHandlers_shared.begin() + ui);

            cout << "cHDF5FileWriter::deregisterCallbackHandler(): Deregistered callback handler: " << pHandler.get() << endl;
            bSuccess = true;
        }
        else
        {
            ui++;
        }
    }

    if(!bSuccess)
    {
        cout << "cHDF5FileWriter::deregisterCallbackHandler(): Warning: Deregistering callback handler: " << pHandler.get() << " failed. Object instance not found." << endl;
    }
}

void cHDF5FileWriter::notifyAllRecordingStarted()
{
    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        m_vpCallbackHandlers[ui]->recordingStarted();
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        m_vpCallbackHandlers[ui]->recordingStarted();
    }
}

void cHDF5FileWriter::notifyAllRecordingStopped()
{
    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        m_vpCallbackHandlers[ui]->recordingStopped();
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        m_vpCallbackHandlers[ui]->recordingStopped();
    }
}
