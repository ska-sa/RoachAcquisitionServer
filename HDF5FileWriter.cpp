
//System includes
#include <sstream>
#include <climits>
#include <ctime>

//Library include
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>

//Local includes
#include "HDF5FileWriter.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"
#include "AVNDataTypes/SpectrometerDataStream/SpectrometerDefinitions.h"

using namespace std;

cHDF5FileWriter::cHDF5FileWriter(const string &strRecordingDirectory, uint32_t u32FileSizeLimit_MB) :
    m_bStreamChanged(false),
    m_strRecordingDirectory(strRecordingDirectory),
    m_u32FileSizeLimit_MB(u32FileSizeLimit_MB),
    m_strFilename(string("Undefined")),
    m_i64LastPrintTime_us(0),
    m_eState(IDLE)
{
    m_oDataStreamInterpreter.registerCallbackHandler(this); //Have data stream handler push interpreted packet data back to this class.

    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsRequestedAzim_us = 0;
        m_oInitialValueSet.m_dVAcsRequestedAzim_deg = 0;
        sprintf(m_oInitialValueSet.m_chaAcsRequestedAzimStatus, "0");

        m_oInitialValueSet.m_i64TSAcsRequestedElev_us = 0;
        m_oInitialValueSet.m_dVAcsRequestedElev_deg = 0;
        sprintf(m_oInitialValueSet.m_chaAcsRequestedElevStatus, "0");

        m_oInitialValueSet.m_i64TSAcsActualAzim_us = 0;
        m_oInitialValueSet.m_dVAcsActualAzim_deg = 0;
        sprintf(m_oInitialValueSet.m_chaAcsActualAzimStatus, "0");

        m_oInitialValueSet.m_i64TSAcsActualElev_us = 0;
        m_oInitialValueSet.m_dVAcsActualElev_deg = 0;
        sprintf( m_oInitialValueSet.m_chaAcsActualElevStatus, "0");

        m_oInitialValueSet.m_i64TSSkyRequestedAzim_us = 0;
        m_oInitialValueSet.m_dVSkyRequestedAzim_deg = 0;
        sprintf(m_oInitialValueSet.m_chaSkyRequestedAzimStatus, "0");

        m_oInitialValueSet.m_i64TSSkyRequestedElev_us = 0;
        m_oInitialValueSet.m_dVSkyRequestedElev_deg = 0;
        sprintf(m_oInitialValueSet.m_chaSkyRequestedElevStatus, "0");

        m_oInitialValueSet.m_i64TSSkyActualAzim_us = 0;
        m_oInitialValueSet.m_dVSkyActualAzim_deg = 0;
        sprintf(m_oInitialValueSet.m_chaSkyActualAzimStatus, "0");

        m_oInitialValueSet.m_i64TSSkyActualElev_us = 0;
        m_oInitialValueSet.m_dVSkyActualElev_deg = 0;
        sprintf(m_oInitialValueSet.m_chaSkyActualElevStatus, "0");

        for (int i = 0; i < 30; i++)
            m_oInitialValueSet.m_aPointingModel[i] = 0;

        m_oInitialValueSet.m_i64TSAntennaStatus_us = 0;
        sprintf(m_oInitialValueSet.m_chaVAntennaStatus, "idle");
        sprintf(m_oInitialValueSet.m_chaAntennaStatusStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverLOFreqIntermediate5GHz_us = 0;
        m_oInitialValueSet.m_dVReceiverLOFreqIntermediate5GHz_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverLOFreqIntermediate5GHzStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverLOFreqIntermediate6_7GHz_us = 0;
        m_oInitialValueSet.m_dVReceiverLOFreqIntermediate6_7GHz_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverLOFreqIntermediate6_7GHzStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverLOFreqFinal_us = 0;
        m_oInitialValueSet.m_dVReceiverLOFreqFinal_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverLOFreqFinalStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverLcpAtten_us = 0;
        m_oInitialValueSet.m_dVReceiverLcpAtten_dB = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverLcpAttenStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverRcpAtten_us = 0;
        m_oInitialValueSet.m_dVReceiverRcpAtten_dB = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverRcpAttenStatus, "0");

        m_oInitialValueSet.m_i64TSFrequencySelectLcp_us = 0;
        m_oInitialValueSet.m_chVFrequencySelectLcp = 0;
        sprintf(m_oInitialValueSet.m_chaFrequencySelectLcpStatus, "0");

        m_oInitialValueSet.m_i64TSFrequencySelectRcp_us = 0;
        m_oInitialValueSet.m_chVFrequencySelectRcp = 0;
        sprintf(m_oInitialValueSet.m_chaFrequencySelectRcpStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiodeInputSource_us = 0;
        sprintf(m_oInitialValueSet.m_chaVNoiseDiodeInputSource, "0");
        sprintf(m_oInitialValueSet.m_chaNoiseDiodeInputSourceStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiodeEnabled_us = 0;
        m_oInitialValueSet.m_bVNoiseDiodeEnabled = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodeEnabledStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiodeSelect_us = 0;
        m_oInitialValueSet.m_i32VNoiseDiodeSelect = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodeSelectStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiodePWMMark_us = 0;
        m_oInitialValueSet.m_i32VNoiseDiodePWMMark = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodePWMMarkStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiodePWMFrequency_us = 0;
        m_oInitialValueSet.m_dVNoiseDiodePWMFrequency_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodePWMFrequencyStatus, "0");

        m_oInitialValueSet.m_i64TSWindSpeed_us = 0;
        m_oInitialValueSet.m_dVWindSpeed_mps = 0;
        sprintf(m_oInitialValueSet.m_chaWindSpeedStatus, "0");

        m_oInitialValueSet.m_i64TSWindDirection_us = 0;
        m_oInitialValueSet.m_dVWindDirection_deg = 0;
        sprintf(m_oInitialValueSet.m_chaWindDirectionStatus, "0");

        m_oInitialValueSet.m_i64TSTemperature_us = 0;
        m_oInitialValueSet.m_dVTemperature_degC = 0;
        sprintf(m_oInitialValueSet.m_chaTemperatureStatus, "0");

        m_oInitialValueSet.m_i64TSAbsolutePressure_us = 0;
        m_oInitialValueSet.m_dVAbsolutePressure_mbar = 0;
        sprintf(m_oInitialValueSet.m_chaAbsolutePressureStatus, "0");

        m_oInitialValueSet.m_i64TSRelativeHumidity_us = 0;
        m_oInitialValueSet.m_dVRelativeHumidity_percent = 0;
        sprintf(m_oInitialValueSet.m_chaRelativeHumidityStatus, "0");
    }
}

cHDF5FileWriter::~cHDF5FileWriter()
{
    stopRecording();

    m_oDataStreamInterpreter.setIsRunning(false);

    cout << "cHDF5FileWriter::~cHDF5FileWriter() exiting." << endl;
}

void cHDF5FileWriter::startRecording(const string &strFilenameSuffix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    if(getState() != IDLE)
    {
        cout << "Warning: HDF5 recording is currently active. Ignoring startRecording request." << endl;
        return;
    }

    cout << "cHDF5FileWriter::startRecording(): Got request to start file recording at " << AVN::stringFromTimestamp_full(i64StartTime_us) << endl;

    m_strFilenameSuffix = strFilenameSuffix;
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
    //cout << "cHDF5FileWriter::setState(): Setting state to " << eState << endl;
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
    //cout << "cHDF5FileWriter::getState(): current state: " << m_eState << endl;
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

        cout << "cHDF5FileWriter::getNextFrame_callback(): Detected stream change in frame update interval. Was " << m_i64FrameInterval_us << " got " <<  oHeader.getTimestamp_us() - m_i64LastTimestamp_us << endl;

        m_i64FrameInterval_us = oHeader.getTimestamp_us() - m_i64LastTimestamp_us;
        m_bStreamChanged = true;
    }
    else
    {
        m_bStreamChanged = false;
    }


    if(m_u32FrameSize_nVal != vi32Chan0.size())
    {
        cout << "cHDF5FileWriter::getNextFrame_callback(): Detected stream change in frame size. Was " << m_u32FrameSize_nVal << " got " << vi32Chan0.size() << endl;

        m_u32FrameSize_nVal = vi32Chan0.size();
        m_bStreamChanged = true;
    }

    if(m_eLastDigitiserType != (AVN::Spectrometer::digitiserType)oHeader.getDigitiserType())
    {
        cout << "cHDF5FileWriter::getNextFrame_callback(): Detected stream change in digitiser. Was \"" << AVN::Spectrometer::digitiserTypeToString(m_eLastDigitiserType)
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
        m_strFilename = makeFilename(m_strRecordingDirectory, m_strFilenameSuffix, m_i64ActualStartTime_us);

        m_pHDF5File = boost::make_shared<cSpectrometerHDF5OutputFile>(m_strFilename, m_eLastDigitiserType, m_u32FrameSize_nVal);

        // TODO: This is a temporary fix to make KatDAL open the file properly, because we're not getting any markup labels
        // from the Field System at the moment.
        m_pHDF5File->addMarkupLabel(time(0)*1e6, "dummy");

        if(m_i64Duration_us)
        {
            m_i64StopTime_us = m_i64ActualStartTime_us + m_i64Duration_us;
        }
        else
        {
            m_i64StopTime_us = LLONG_MAX; //0 duration implies record indefinitely.
        }

        // Add any initial values stored.
        if (m_oInitialValueSet.m_i64TSAcsRequestedAzim_us)
            m_pHDF5File->addAcsRequestedAz(m_oInitialValueSet.m_i64TSAcsRequestedAzim_us,
                                                  m_oInitialValueSet.m_dVAcsRequestedAzim_deg,
                                                  m_oInitialValueSet.m_chaAcsRequestedAzimStatus);
        if (m_oInitialValueSet.m_i64TSAcsRequestedElev_us)
            m_pHDF5File->addAcsRequestedEl(m_oInitialValueSet.m_i64TSAcsRequestedElev_us,
                                           m_oInitialValueSet.m_dVAcsRequestedElev_deg,
                                           m_oInitialValueSet.m_chaAcsRequestedElevStatus);
        if (m_oInitialValueSet.m_i64TSAcsActualAzim_us)
            m_pHDF5File->addAcsActualAz(m_oInitialValueSet.m_i64TSAcsActualAzim_us,
                                        m_oInitialValueSet.m_dVAcsActualAzim_deg,
                                        m_oInitialValueSet.m_chaAcsActualAzimStatus);
        if ( m_oInitialValueSet.m_i64TSAcsActualElev_us)
            m_pHDF5File->addAcsActualEl(m_oInitialValueSet.m_i64TSAcsActualElev_us,
                                        m_oInitialValueSet.m_dVAcsActualElev_deg,
                                        m_oInitialValueSet.m_chaAcsActualElevStatus);
        if (m_oInitialValueSet.m_i64TSSkyRequestedAzim_us)
            m_pHDF5File->addSkyRequestedAz(m_oInitialValueSet.m_i64TSSkyRequestedAzim_us,
                                           m_oInitialValueSet.m_dVSkyRequestedAzim_deg,
                                           m_oInitialValueSet.m_chaSkyRequestedAzimStatus);
        if (m_oInitialValueSet.m_i64TSSkyRequestedElev_us)
            m_pHDF5File->addSkyRequestedEl(m_oInitialValueSet.m_i64TSSkyRequestedElev_us,
                                           m_oInitialValueSet.m_dVSkyRequestedElev_deg,
                                           m_oInitialValueSet.m_chaSkyRequestedElevStatus);
        if (m_oInitialValueSet.m_i64TSSkyActualAzim_us)
            m_pHDF5File->addSkyActualAz(m_oInitialValueSet.m_i64TSSkyActualAzim_us,
                                        m_oInitialValueSet.m_dVSkyActualAzim_deg,
                                        m_oInitialValueSet.m_chaSkyActualAzimStatus);
        if (m_oInitialValueSet.m_i64TSSkyActualElev_us)
            m_pHDF5File->addSkyActualEl(m_oInitialValueSet.m_i64TSSkyActualElev_us,
                                        m_oInitialValueSet.m_dVSkyActualElev_deg,
                                        m_oInitialValueSet.m_chaSkyActualElevStatus);
        for (int i = 0; i < 30; i++)
            m_pHDF5File->addPointingModelParameter(i, m_oInitialValueSet.m_aPointingModel[i]);
        if (m_oInitialValueSet.m_i64TSAntennaStatus_us)
            m_pHDF5File->addAntennaStatus(m_oInitialValueSet.m_i64TSAntennaStatus_us,
                                          m_oInitialValueSet.m_chaVAntennaStatus,
                                          m_oInitialValueSet.m_chaAntennaStatusStatus);
        if (m_oInitialValueSet.m_i64TSReceiverLOFreqIntermediate5GHz_us)
            m_pHDF5File->addFrequencyLOIntermed5GHz(m_oInitialValueSet.m_i64TSReceiverLOFreqIntermediate5GHz_us,
                                                        m_oInitialValueSet.m_dVReceiverLOFreqIntermediate5GHz_Hz,
                                                        m_oInitialValueSet.m_chaReceiverLOFreqIntermediate5GHzStatus);
        if (m_oInitialValueSet.m_i64TSReceiverLOFreqIntermediate6_7GHz_us)
            m_pHDF5File->addFrequencyLOIntermed6_7GHz(m_oInitialValueSet.m_i64TSReceiverLOFreqIntermediate6_7GHz_us,
                                                          m_oInitialValueSet.m_dVReceiverLOFreqIntermediate6_7GHz_Hz,
                                                          m_oInitialValueSet.m_chaReceiverLOFreqIntermediate6_7GHzStatus);
        if (m_oInitialValueSet.m_i64TSReceiverLOFreqFinal_us)
            m_pHDF5File->addFrequencyLOFinal(m_oInitialValueSet.m_i64TSReceiverLOFreqFinal_us,
                                             m_oInitialValueSet.m_dVReceiverLOFreqFinal_Hz,
                                             m_oInitialValueSet.m_chaReceiverLOFreqFinalStatus);
        if (m_oInitialValueSet.m_i64TSReceiverLcpAtten_us)
            m_pHDF5File->addReceiverLcpAttenuation(m_oInitialValueSet.m_i64TSReceiverLcpAtten_us,
                                                   m_oInitialValueSet.m_dVReceiverLcpAtten_dB,
                                                   m_oInitialValueSet.m_chaReceiverLcpAttenStatus);
        if (m_oInitialValueSet.m_i64TSReceiverRcpAtten_us)
            m_pHDF5File->addReceiverRcpAttenuation(m_oInitialValueSet.m_i64TSReceiverRcpAtten_us,
                                                   m_oInitialValueSet.m_dVReceiverRcpAtten_dB,
                                                   m_oInitialValueSet.m_chaReceiverRcpAttenStatus);
        if (m_oInitialValueSet.m_i64TSFrequencySelectLcp_us)
            m_pHDF5File->addFrequencySelectLcp(m_oInitialValueSet.m_i64TSFrequencySelectLcp_us,
                                               m_oInitialValueSet.m_chVFrequencySelectLcp,
                                               m_oInitialValueSet.m_chaFrequencySelectLcpStatus);
        if (m_oInitialValueSet.m_i64TSFrequencySelectRcp_us)
            m_pHDF5File->addFrequencySelectLcp(m_oInitialValueSet.m_i64TSFrequencySelectRcp_us,
                                               m_oInitialValueSet.m_chVFrequencySelectRcp,
                                               m_oInitialValueSet.m_chaFrequencySelectRcpStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiodeInputSource_us)
            m_pHDF5File->addNoiseDiodeInputSource(m_oInitialValueSet.m_i64TSNoiseDiodeInputSource_us,
                                                  m_oInitialValueSet.m_chaVNoiseDiodeInputSource,
                                                  m_oInitialValueSet.m_chaNoiseDiodeInputSourceStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiodeEnabled_us)
            m_pHDF5File->addNoiseDiodeEnable(m_oInitialValueSet.m_i64TSNoiseDiodeEnabled_us,
                                             m_oInitialValueSet.m_bVNoiseDiodeEnabled,
                                             m_oInitialValueSet.m_chaNoiseDiodeEnabledStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiodeSelect_us)
            m_pHDF5File->addNoiseDiodeSelect(m_oInitialValueSet.m_i64TSNoiseDiodeSelect_us,
                                             m_oInitialValueSet.m_i32VNoiseDiodeSelect,
                                             m_oInitialValueSet.m_chaNoiseDiodeSelectStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiodePWMMark_us)
            m_pHDF5File->addNoiseDiodePWMMark(m_oInitialValueSet.m_i64TSNoiseDiodePWMMark_us,
                                              m_oInitialValueSet.m_i32VNoiseDiodePWMMark,
                                              m_oInitialValueSet.m_chaNoiseDiodePWMMarkStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiodePWMFrequency_us)
            m_pHDF5File->addNoiseDiodePWMFrequency(m_oInitialValueSet.m_i64TSNoiseDiodePWMFrequency_us,
                                                   m_oInitialValueSet.m_dVNoiseDiodePWMFrequency_Hz,
                                                   m_oInitialValueSet.m_chaNoiseDiodePWMFrequencyStatus);
        if (m_oInitialValueSet.m_i64TSWindSpeed_us)
            m_pHDF5File->addWindSpeed(m_oInitialValueSet.m_i64TSWindSpeed_us,
                                      m_oInitialValueSet.m_dVWindSpeed_mps,
                                      m_oInitialValueSet.m_chaWindSpeedStatus);
        if (m_oInitialValueSet.m_i64TSWindDirection_us)
            m_pHDF5File->addWindDirection(m_oInitialValueSet.m_i64TSWindDirection_us,
                                          m_oInitialValueSet.m_dVWindDirection_deg,
                                          m_oInitialValueSet.m_chaWindDirectionStatus);
        if (m_oInitialValueSet.m_i64TSTemperature_us)
            m_pHDF5File->addTemperature(m_oInitialValueSet.m_i64TSTemperature_us,
                                        m_oInitialValueSet.m_dVTemperature_degC,
                                        m_oInitialValueSet.m_chaTemperatureStatus);
        if (m_oInitialValueSet.m_i64TSAbsolutePressure_us)
            m_pHDF5File->addAbsolutePressure(m_oInitialValueSet.m_i64TSAbsolutePressure_us,
                                             m_oInitialValueSet.m_dVAbsolutePressure_mbar,
                                             m_oInitialValueSet.m_chaAbsolutePressureStatus);
        if (m_oInitialValueSet.m_i64TSRelativeHumidity_us)
            m_pHDF5File->addRelativeHumidity(m_oInitialValueSet.m_i64TSRelativeHumidity_us,
                                             m_oInitialValueSet.m_dVRelativeHumidity_percent,
                                             m_oInitialValueSet.m_chaRelativeHumidityStatus);

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
            setState(REQUEST_START);
            m_pHDF5File.reset(); //Closes current HDF5 file.
            break;

            //The current (first different) frame will be lost. At present this seems like acceptable behavior.
            //If not this code will need to jump back to the previous state instead of exiting the function. This may
            //require more flags and a loop or doing this check above the switch.
        }

        //Otherwise write the frame to file...
        m_pHDF5File->addFrame(vi32Chan0, vi32Chan1, vi32Chan2, vi32Chan3, oHeader);

        if(m_i64LastTimestamp_us - m_i64LastPrintTime_us > 500000) //Limit this output to ever 500 ms
        {
            //Get the filesize so far
            double dFileSize_MB = getCurrentFileSize_MB();

            if(m_i64StopTime_us == LLONG_MAX)
            {
                cout << "Last timestamp : " << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us)
                     << " | Elapsed time: " << AVN::stringFromTimeDuration(m_i64LastTimestamp_us - m_i64ActualStartTime_us)
                     << " | File size: " << dFileSize_MB << " MB"
                     << "\r" << std::flush;
            }
            else
            {
                cout << "Last timestamp : " << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us)
                     << " | Elapsed time: " << AVN::stringFromTimeDuration(m_i64LastTimestamp_us - m_i64ActualStartTime_us)
                     << " | Time to stop : " << AVN::stringFromTimeDuration(m_i64StopTime_us - m_i64LastTimestamp_us)
                     << " | File size: " << dFileSize_MB << " MB"
                     << "\r" << std::flush;
            }

            m_i64LastPrintTime_us = m_i64LastTimestamp_us;

            //Check the filesize vs the specified limit (Limit of zero implies no limit)
            if(m_u32FileSizeLimit_MB && dFileSize_MB > (double)m_u32FileSizeLimit_MB)
            {
                cout << "cHDF5FileWriter::getNextFrame_callback(): Filesize has exceeded specified size limit. Closing and start a new one." << endl;
                setState(REQUEST_START);
                m_pHDF5File.reset(); //Closes current HDF5 file.
                break;
            }

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

std::string cHDF5FileWriter::makeFilename(const std::string &strDirectory, const std::string &strSuffix, int64_t i64Timestamp_us)
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

    oSS << AVN::stringFromTimestamp_full(m_i64LastTimestamp_us); //Note we use the actual packet timestamp and not the user requested start time.
    oSS << string("_");
    oSS << m_strFilenameSuffix;
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
std::string cHDF5FileWriter::getRecordingDirectory()
{
    boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

    return m_strRecordingDirectory;
}

uint64_t cHDF5FileWriter::getCurrentFileSize_B()
{
    if(getState() == RECORDING)
    {
        return (uint64_t)boost::filesystem::file_size(m_pHDF5File->getFilename());
    }
    else
    {
        return 0;
    }

}

double cHDF5FileWriter::getCurrentFileSize_MB()
{
    return (double)getCurrentFileSize_B() / 1e6;
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
        m_vpCallbackHandlers[ui]->recordingStarted_callback();
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        m_vpCallbackHandlers_shared[ui]->recordingStarted_callback();
    }
}

void cHDF5FileWriter::notifyAllRecordingStopped()
{
    for(uint32_t ui = 0; ui < m_vpCallbackHandlers.size(); ui++)
    {
        m_vpCallbackHandlers[ui]->recordingStopped_callback();
    }

    for(uint32_t ui = 0; ui < m_vpCallbackHandlers_shared.size(); ui++)
    {
        m_vpCallbackHandlers_shared[ui]->recordingStopped_callback();
    }
}

//Callbacks implement for KATCPClient callback interface. These pass information to be stored to HDF5 file:
void cHDF5FileWriter::connected_callback(bool bConnected, const std::string &strHostAddress, uint16_t u16Port, const std::string &strDescription)
{
    //Not used
}

void cHDF5FileWriter::startRecording_callback(const std::string &strFileSuffix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    cout << "--------------------------------------------------------------" << endl;
    cout << "cHDF5FileWriter::startRecording_callback Got request to record via KATCPClient callback:" << endl;
    cout << "File suffix = " << strFileSuffix << endl;
    cout << "Start time  = " << i64StartTime_us << " (" << AVN::stringFromTimestamp_full(i64StartTime_us) << ")" << endl;
    cout << "Duration    = " << i64Duration_us << " (" << AVN::stringFromTimeDuration(i64Duration_us) << ")" << endl;
    cout << "--------------------------------------------------------------" << endl;

    startRecording(strFileSuffix, i64StartTime_us, i64Duration_us);
}

void cHDF5FileWriter::stopRecording_callback()
{
    stopRecording();
}


void cHDF5FileWriter::acsRequestedAz_callback(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAcsRequestedAzim_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsRequestedAzim_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAcsRequestedAzim_deg = dAzimuth_deg;
        sprintf(m_oInitialValueSet.m_chaAcsRequestedAzimStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAcsRequestedAz(i64Timestamp_us, dAzimuth_deg, strStatus);
}

void cHDF5FileWriter::acsRequestedEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAcsRequestedElev_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsRequestedElev_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAcsRequestedElev_deg = dElevation_deg;
        sprintf(m_oInitialValueSet.m_chaAcsRequestedElevStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAcsRequestedEl(i64Timestamp_us, dElevation_deg, strStatus);
}

void cHDF5FileWriter::acsActualAz_callback(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAcsActualAzim_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsActualAzim_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAcsActualAzim_deg = dAzimuth_deg;
        sprintf(m_oInitialValueSet.m_chaAcsActualAzimStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAcsActualAz(i64Timestamp_us, dAzimuth_deg, strStatus);
}

void cHDF5FileWriter::acsActualEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAcsActualElev_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsActualElev_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAcsActualElev_deg = dElevation_deg;
        sprintf(m_oInitialValueSet.m_chaAcsActualElevStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAcsActualEl(i64Timestamp_us, dElevation_deg, strStatus);
}

void cHDF5FileWriter::skyRequestedAz_callback(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSSkyRequestedAzim_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSSkyRequestedAzim_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVSkyRequestedAzim_deg = dAzimuth_deg;
        sprintf(m_oInitialValueSet.m_chaSkyRequestedAzimStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addSkyRequestedAz(i64Timestamp_us, dAzimuth_deg, strStatus);
}

void cHDF5FileWriter::skyRequestedEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSSkyRequestedElev_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSSkyRequestedElev_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVSkyRequestedElev_deg = dElevation_deg;
        sprintf(m_oInitialValueSet.m_chaSkyRequestedElevStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addSkyRequestedEl(i64Timestamp_us, dElevation_deg, strStatus);
}

void cHDF5FileWriter::skyActualAz_callback(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSSkyActualAzim_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSSkyActualAzim_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVSkyActualAzim_deg = dAzimuth_deg;
        sprintf(m_oInitialValueSet.m_chaSkyActualAzimStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addSkyActualAz(i64Timestamp_us, dAzimuth_deg, strStatus);
}

void cHDF5FileWriter::skyActualEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSSkyActualElev_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSSkyActualElev_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVSkyActualElev_deg = dElevation_deg;
        sprintf(m_oInitialValueSet.m_chaSkyActualElevStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addSkyActualEl(i64Timestamp_us, dElevation_deg, strStatus);
}

void cHDF5FileWriter::pointingModel_callback(uint8_t ui8ParameterNumber, double dParameterValue)
{
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_aPointingModel[ui8ParameterNumber - 1] = dParameterValue;
    }
    if (getState() == RECORDING)
        m_pHDF5File->addPointingModelParameter(ui8ParameterNumber - 1, dParameterValue);
}

void cHDF5FileWriter::antennaStatus_callback(int64_t i64Timestamp_us, const string &strAntennaStatus, const string &strStatus)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addAntennaStatus(i64Timestamp_us, strAntennaStatus, strStatus);
}

void cHDF5FileWriter::appliedPointingModel_callback(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    if(getState() != RECORDING)
        return;

    //m_pHDF5File->setAppliedPointingModel(strModelName, vdPointingModelParams);
}

void cHDF5FileWriter::antennaName_callback(const string &strAntennaName)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setAntennaName(strAntennaName);
}

void cHDF5FileWriter::antennaDiameter_callback(const string &strAntennaDiameter)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setAntennaBeamwidth(strAntennaDiameter);
}

void cHDF5FileWriter::antennaBeamwidth_callback(const string &strAntennaBeamwidth)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setAntennaBeamwidth(strAntennaBeamwidth);
}

void cHDF5FileWriter::antennaLongitude_callback(const string &strAntennaLongitude)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setAntennaLongitude(strAntennaLongitude);
}

void cHDF5FileWriter::antennaLatitude_callback(const string &strAntennaLatitude)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setAntennaLatitude(strAntennaLatitude);
}

void cHDF5FileWriter::rNoiseDiodeInputSource_callback(int64_t i64Timestamp_us, const string &strInputSource, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiodeInputSource_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiodeInputSource_us = i64Timestamp_us;
        sprintf(m_oInitialValueSet.m_chaVNoiseDiodeInputSource, "%s", strInputSource.c_str());
        sprintf(m_oInitialValueSet.m_chaNoiseDiodeInputSourceStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiodeInputSource(i64Timestamp_us, strInputSource, strStatus);
}

void cHDF5FileWriter::rNoiseDiodeEnabled_callback(int64_t i64Timestamp_us, bool bNoiseDiodeEnabled, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiodeEnabled_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiodeEnabled_us = i64Timestamp_us;
        m_oInitialValueSet.m_bVNoiseDiodeEnabled = bNoiseDiodeEnabled;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodeEnabledStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiodeEnable(i64Timestamp_us, bNoiseDiodeEnabled, strStatus);
}

void cHDF5FileWriter::rNoiseDiodeSelect_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeSelect, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiodeSelect_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiodeSelect_us = i64Timestamp_us;
        m_oInitialValueSet.m_i32VNoiseDiodeSelect = i32NoiseDiodeSelect;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodeSelectStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiodeSelect(i64Timestamp_us, i32NoiseDiodeSelect, strStatus);
}

void cHDF5FileWriter::rNoiseDiodePWMMark_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodePWMMark, const string &strStatus)
{
    if (getState() != RECORDING)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiodePWMMark_us = i64Timestamp_us;
        m_oInitialValueSet.m_i32VNoiseDiodePWMMark = i32NoiseDiodePWMMark;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodePWMMarkStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiodePWMMark(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
}

void cHDF5FileWriter::rNoiseDiodePWMFrequency_callback(int64_t i64Timestamp_us, double dNoiseDiodePWMFrequency, const string &strStatus)
{
    if (getState() != RECORDING)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiodePWMFrequency_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVNoiseDiodePWMFrequency_Hz = dNoiseDiodePWMFrequency;
        sprintf(m_oInitialValueSet.m_chaNoiseDiodePWMFrequencyStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiodePWMFrequency(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
}

void cHDF5FileWriter::sourceSelection_callback(int64_t i64Timestamp_us, const string &strSourceName, double dRighAscension_deg, double dDeclination_deg)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addSourceSelection(i64Timestamp_us, strSourceName, dRighAscension_deg, dDeclination_deg);
}

void cHDF5FileWriter::frequencySelectLcp_callback(int64_t i64Timestamp_us, bool bFrequencySelectLcp, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSFrequencySelectLcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSFrequencySelectLcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_chVFrequencySelectLcp = bFrequencySelectLcp;
        sprintf(m_oInitialValueSet.m_chaFrequencySelectLcpStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addFrequencySelectLcp(i64Timestamp_us, bFrequencySelectLcp, strStatus);
}

void cHDF5FileWriter::frequencySelectRcp_callback(int64_t i64Timestamp_us, bool bFrequencySelectRcp, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSFrequencySelectRcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSFrequencySelectRcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_chVFrequencySelectRcp = bFrequencySelectRcp;
        sprintf(m_oInitialValueSet.m_chaFrequencySelectRcpStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addFrequencySelectRcp(i64Timestamp_us, bFrequencySelectRcp, strStatus);
}

void cHDF5FileWriter::frequencyLOIntermediate5GHz_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan0_Hz, const string &strStatus)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addFrequencyLOIntermed5GHz(i64Timestamp_us, dFrequencyLO0Chan0_Hz, strStatus);
}

void cHDF5FileWriter::frequencyLOIntermediate6_7GHz_callback(int64_t i64Timestamp_us, double dFrequencyLO0Chan1_Hz, const string &strStatus)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addFrequencyLOIntermed6_7GHz(i64Timestamp_us, dFrequencyLO0Chan1_Hz, strStatus);
}

void cHDF5FileWriter::frequencyLOFinal_callback(int64_t i64Timestamp_us, double dFrequencyLO1_Hz, const string &strStatus)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addFrequencyLOFinal(i64Timestamp_us, dFrequencyLO1_Hz, strStatus);
}

void cHDF5FileWriter::receiverBandwidthLcp_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan0_Hz, const string &strStatus)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addReceiverBandwidthLcp(i64Timestamp_us, dReceiverBandwidthChan0_Hz, strStatus);
}

void cHDF5FileWriter::receiverBandwidthRcp_callback(int64_t i64Timestamp_us, double dReceiverBandwidthChan1_Hz, const string &strStatus)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addReceiverBandwidthRcp(i64Timestamp_us, dReceiverBandwidthChan1_Hz, strStatus);
}

void cHDF5FileWriter::receiverLcpAttenuation_callback(int64_t i64Timestamp_us, double dReceiverLcpAttenuation_dB, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverLcpAtten_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverLcpAtten_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverLcpAtten_dB = dReceiverLcpAttenuation_dB;
        sprintf(m_oInitialValueSet.m_chaReceiverLcpAttenStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addReceiverLcpAttenuation(i64Timestamp_us, dReceiverLcpAttenuation_dB, strStatus);
}

void cHDF5FileWriter::receiverRcpAttenuation_callback(int64_t i64Timestamp_us, double dReceiverRcpAttenuation_dB, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverRcpAtten_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverRcpAtten_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverRcpAtten_dB = dReceiverRcpAttenuation_dB;
        sprintf(m_oInitialValueSet.m_chaReceiverRcpAttenStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addReceiverRcpAttenuation(i64Timestamp_us, dReceiverRcpAttenuation_dB, strStatus);
}

void cHDF5FileWriter::envWindSpeed_callback(int64_t i64Timestamp_us, double dWindSpeed_mps, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSWindSpeed_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSWindSpeed_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVWindSpeed_mps = dWindSpeed_mps;
        sprintf(m_oInitialValueSet.m_chaWindSpeedStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addWindSpeed(i64Timestamp_us, dWindSpeed_mps, strStatus);
}

void cHDF5FileWriter::envWindDirection_callback(int64_t i64Timestamp_us, double dWindDirection_degrees, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSWindDirection_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSWindDirection_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVWindDirection_deg = dWindDirection_degrees;
        sprintf(m_oInitialValueSet.m_chaWindDirectionStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addWindDirection(i64Timestamp_us, dWindDirection_degrees, strStatus);
}

void cHDF5FileWriter::envTemperature_callback(int64_t i64Timestamp_us, double dTemperature_degreesC, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSTemperature_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSTemperature_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVTemperature_degC = dTemperature_degreesC;
        sprintf(m_oInitialValueSet.m_chaTemperatureStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addTemperature(i64Timestamp_us, dTemperature_degreesC, strStatus);
}

void cHDF5FileWriter::envAbsolutePressure_callback(int64_t i64Timestamp_us, double dPressure_mbar, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAbsolutePressure_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAbsolutePressure_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAbsolutePressure_mbar = dPressure_mbar;
        sprintf(m_oInitialValueSet.m_chaAbsolutePressureStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAbsolutePressure(i64Timestamp_us, dPressure_mbar, strStatus);
}

void cHDF5FileWriter::envRelativeHumidity_callback(int64_t i64Timestamp_us, double dHumidity_percent, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSRelativeHumidity_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSRelativeHumidity_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVRelativeHumidity_percent = dHumidity_percent;
        sprintf(m_oInitialValueSet.m_chaRelativeHumidityStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addRelativeHumidity(i64Timestamp_us, dHumidity_percent, strStatus);
}

void cHDF5FileWriter::accumulationLength_callback(int64_t i64Timestamp_us, uint32_t u32NSamples)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addAccumulationLength(i64Timestamp_us, u32NSamples);
}

void cHDF5FileWriter::coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addCoarseChannelSelect(i64Timestamp_us, u32ChannelNo);
}

void cHDF5FileWriter::frequencyFs_callback(double dFrequencyFs_Hz)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setFrequencyFs(dFrequencyFs_Hz);
}

void cHDF5FileWriter::sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setSizeOfCoarseFFT(u32SizeOfCoarseFFT_nSamp);
}

void cHDF5FileWriter::sizeOfFineFFT_callback(uint32_t u32SizeOfFineFFT_nSamp)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->setSizeOfFineFFT(u32SizeOfFineFFT_nSamp);
}

void cHDF5FileWriter::coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addCoarseFFTShiftMask(i64Timestamp_us, u32ShiftMask);
}

void cHDF5FileWriter::attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addAttenuationADCChan0(i64Timestamp_us, dADCAttenuationChan0_dB);
}

void cHDF5FileWriter::attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB)
{
    if(getState() != RECORDING)
        return;

    m_pHDF5File->addAttenuationADCChan1(i64Timestamp_us, dADCAttenuationChan1_dB);
}

void cHDF5FileWriter::overflowsRegs_callback(int64_t i64Timestamp_us, uint32_t u32OverflowRegs)
{
    //TODO
}

void cHDF5FileWriter::eth10GbEUp_callback(int64_t i64Timestamp_us, bool bEth10GbEUp)
{
    //TODO
}

void cHDF5FileWriter::ppsCount_callback(int64_t i64Timestamp_us, uint32_t u32PPSCount)
{
    //TODO
}

void cHDF5FileWriter::clockFrequency_callback(int64_t i64Timestamp_us, uint32_t u32ClockFrequency_Hz)
{
    //TODO
}

