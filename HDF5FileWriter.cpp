
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

        m_oInitialValueSet.m_i64TSAcsDesiredAzim_us = 0;
        m_oInitialValueSet.m_dVAcsDesiredAzim_deg = 0;
        sprintf(m_oInitialValueSet.m_chaAcsDesiredAzimStatus, "0");

        m_oInitialValueSet.m_i64TSAcsDesiredElev_us = 0;
        m_oInitialValueSet.m_dVAcsDesiredElev_deg = 0;
        sprintf(m_oInitialValueSet.m_chaAcsDesiredElevStatus, "0");

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

        m_oInitialValueSet.m_i64TSSkyDesiredAzim_us = 0;
        m_oInitialValueSet.m_dVSkyDesiredAzim_deg = 0;
        sprintf(m_oInitialValueSet.m_chaSkyDesiredAzimStatus, "0");

        m_oInitialValueSet.m_i64TSSkyDesiredElev_us = 0;
        m_oInitialValueSet.m_dVSkyDesiredElev_deg = 0;
        sprintf(m_oInitialValueSet.m_chaSkyDesiredElevStatus, "0");

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

        m_oInitialValueSet.m_i64TSReceiverSkyFreq5GHz_us = 0;
        m_oInitialValueSet.m_dVReceiverSkyFreq5GHz_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverSkyFreq5GHzStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverSkyFreq6_7GHz_us = 0;
        m_oInitialValueSet.m_dVReceiverSkyFreq6_7GHz_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverSkyFreq6_7GHzStatus, "0");

        m_oInitialValueSet.m_i64TSReceiverGain5GHzLcp_us = 0;
        m_oInitialValueSet.m_dVReceiverGain5GHzLcp = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverGain5GHzLcp, "0");

        m_oInitialValueSet.m_i64TSReceiverGain5GHzRcp_us = 0;
        m_oInitialValueSet.m_dVReceiverGain5GHzRcp = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverGain5GHzRcp, "0");

        m_oInitialValueSet.m_i64TSReceiverGain6_7GHzLcp_us = 0;
        m_oInitialValueSet.m_dVReceiverGain6_7GHzLcp = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverGain6_7GHzLcp, "0");

        m_oInitialValueSet.m_i64TSReceiverGain6_7GHzRcp_us = 0;
        m_oInitialValueSet.m_dVReceiverGain6_7GHzRcp = 0;
        sprintf(m_oInitialValueSet.m_chaReceiverGain6_7GHzRcp, "0");

        m_oInitialValueSet.m_i64TSBandSelectLcp_us = 0;
        m_oInitialValueSet.m_chVBandSelectLcp = 0;
        sprintf(m_oInitialValueSet.m_chaBandSelectLcpStatus, "0");

        m_oInitialValueSet.m_i64TSBandSelectRcp_us = 0;
        m_oInitialValueSet.m_chVBandSelectRcp = 0;
        sprintf(m_oInitialValueSet.m_chaBandSelectRcpStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode5GHzInputSource_us = 0;
        sprintf(m_oInitialValueSet.m_chaVNoiseDiode5GHzInputSource, "0");
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzInputSourceStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode5GHzLevel_us = 0;
        m_oInitialValueSet.m_i32VNoiseDiode5GHzLevel = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzLevelStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMMark_us = 0;
        m_oInitialValueSet.m_i32VNoiseDiode5GHzPWMMark = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzPWMMarkStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMFrequency_us = 0;
        m_oInitialValueSet.m_dVNoiseDiode5GHzPWMFrequency_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzPWMFrequencyStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzInputSource_us = 0;
        sprintf(m_oInitialValueSet.m_chaVNoiseDiode6_7GHzInputSource, "0");
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzInputSourceStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzLevel_us = 0;
        m_oInitialValueSet.m_i32VNoiseDiode6_7GHzLevel = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzLevelStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMMark_us = 0;
        m_oInitialValueSet.m_i32VNoiseDiode6_7GHzPWMMark = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzPWMMarkStatus, "0");

        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMFrequency_us = 0;
        m_oInitialValueSet.m_dVNoiseDiode6_7GHzPWMFrequency_Hz = 0;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzPWMFrequencyStatus, "0");

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

        m_oInitialValueSet.m_i64TSAccumulationLength_us = 0;
        m_oInitialValueSet.m_u32VAccumulationLength_frames = 0;

        m_oInitialValueSet.m_i64TSCoarseChannelSelect_us = 0;
        m_oInitialValueSet.m_u32VCoarseChannelSelect = 0;

        m_oInitialValueSet.m_i64TSCoarseFFTShiftMask_us = 0;
        m_oInitialValueSet.m_u32VCoarseFFTShiftMask = 0;

        m_oInitialValueSet.m_i64TSDspGain_us = 0;
        m_oInitialValueSet.m_dVDspGain = 0;

        m_oInitialValueSet.m_i64TSAttenuationADCChan0_us = 0;
        m_oInitialValueSet.m_dVAttenuationADCChan0_dB = 0;

        m_oInitialValueSet.m_i64TSAttenuationADCChan1_us = 0;
        m_oInitialValueSet.m_dVAttenuationADCChan1_dB = 0;
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
        if (m_oInitialValueSet.m_i64TSAcsDesiredAzim_us)
            m_pHDF5File->addAcsDesiredAz(m_oInitialValueSet.m_i64TSAcsDesiredAzim_us,
                                         m_oInitialValueSet.m_dVAcsDesiredAzim_deg,
                                         m_oInitialValueSet.m_chaAcsDesiredAzimStatus);
        if (m_oInitialValueSet.m_i64TSAcsDesiredElev_us)
            m_pHDF5File->addAcsDesiredEl(m_oInitialValueSet.m_i64TSAcsDesiredElev_us,
                                         m_oInitialValueSet.m_dVAcsDesiredElev_deg,
                                         m_oInitialValueSet.m_chaAcsDesiredElevStatus);
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
        if (m_oInitialValueSet.m_i64TSSkyDesiredAzim_us)
            m_pHDF5File->addSkyDesiredAz(m_oInitialValueSet.m_i64TSSkyDesiredAzim_us,
                                         m_oInitialValueSet.m_dVSkyDesiredAzim_deg,
                                         m_oInitialValueSet.m_chaSkyDesiredAzimStatus);
        if (m_oInitialValueSet.m_i64TSSkyDesiredElev_us)
            m_pHDF5File->addSkyDesiredEl(m_oInitialValueSet.m_i64TSSkyDesiredElev_us,
                                         m_oInitialValueSet.m_dVSkyDesiredElev_deg,
                                         m_oInitialValueSet.m_chaSkyDesiredElevStatus);
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
        if (m_oInitialValueSet.m_i64TSReceiverSkyFreq5GHz_us)
            m_pHDF5File->addFrequencySky5GHz(m_oInitialValueSet.m_i64TSReceiverSkyFreq5GHz_us,
                                             m_oInitialValueSet.m_dVReceiverSkyFreq5GHz_Hz,
                                             m_oInitialValueSet.m_chaReceiverSkyFreq5GHzStatus);
        if (m_oInitialValueSet.m_i64TSReceiverSkyFreq6_7GHz_us)
            m_pHDF5File->addFrequencySky6_7GHz(m_oInitialValueSet.m_i64TSReceiverSkyFreq6_7GHz_us,
                                               m_oInitialValueSet.m_dVReceiverSkyFreq6_7GHz_Hz,
                                               m_oInitialValueSet.m_chaReceiverSkyFreq6_7GHzStatus);
        if (m_oInitialValueSet.m_i64TSReceiverGain5GHzLcp_us)
            m_pHDF5File->addReceiverGain5GHzLcp(m_oInitialValueSet.m_i64TSReceiverGain5GHzLcp_us,
                                                   m_oInitialValueSet.m_dVReceiverGain5GHzLcp,
                                                   m_oInitialValueSet.m_chaReceiverGain5GHzLcp);
        if (m_oInitialValueSet.m_i64TSReceiverGain5GHzRcp_us)
            m_pHDF5File->addReceiverGain5GHzRcp(m_oInitialValueSet.m_i64TSReceiverGain5GHzRcp_us,
                                                   m_oInitialValueSet.m_dVReceiverGain5GHzRcp,
                                                   m_oInitialValueSet.m_chaReceiverGain5GHzRcp);
        if (m_oInitialValueSet.m_i64TSReceiverGain6_7GHzLcp_us)
            m_pHDF5File->addReceiverGain6_7GHzLcp(m_oInitialValueSet.m_i64TSReceiverGain6_7GHzLcp_us,
                                                   m_oInitialValueSet.m_dVReceiverGain6_7GHzLcp,
                                                   m_oInitialValueSet.m_chaReceiverGain6_7GHzLcp);
        if (m_oInitialValueSet.m_i64TSReceiverGain6_7GHzRcp_us)
            m_pHDF5File->addReceiverGain6_7GHzRcp(m_oInitialValueSet.m_i64TSReceiverGain6_7GHzRcp_us,
                                                   m_oInitialValueSet.m_dVReceiverGain6_7GHzRcp,
                                                   m_oInitialValueSet.m_chaReceiverGain6_7GHzRcp);
        if (m_oInitialValueSet.m_i64TSBandSelectLcp_us)
            m_pHDF5File->addBandSelectLcp(m_oInitialValueSet.m_i64TSBandSelectLcp_us,
                                               m_oInitialValueSet.m_chVBandSelectLcp,
                                               m_oInitialValueSet.m_chaBandSelectLcpStatus);
        if (m_oInitialValueSet.m_i64TSBandSelectRcp_us)
            m_pHDF5File->addBandSelectRcp(m_oInitialValueSet.m_i64TSBandSelectRcp_us,
                                               m_oInitialValueSet.m_chVBandSelectRcp,
                                               m_oInitialValueSet.m_chaBandSelectRcpStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode5GHzInputSource_us)
            m_pHDF5File->addNoiseDiode5GHzInputSource(m_oInitialValueSet.m_i64TSNoiseDiode5GHzInputSource_us,
                                                  m_oInitialValueSet.m_chaVNoiseDiode5GHzInputSource,
                                                  m_oInitialValueSet.m_chaNoiseDiode5GHzInputSourceStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode5GHzLevel_us)
            m_pHDF5File->addNoiseDiode5GHzLevel(m_oInitialValueSet.m_i64TSNoiseDiode5GHzLevel_us,
                                             m_oInitialValueSet.m_i32VNoiseDiode5GHzLevel,
                                             m_oInitialValueSet.m_chaNoiseDiode5GHzLevelStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMMark_us)
            m_pHDF5File->addNoiseDiode5GHzPWMMark(m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMMark_us,
                                              m_oInitialValueSet.m_i32VNoiseDiode5GHzPWMMark,
                                              m_oInitialValueSet.m_chaNoiseDiode5GHzPWMMarkStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMFrequency_us)
            m_pHDF5File->addNoiseDiode5GHzPWMFrequency(m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMFrequency_us,
                                                   m_oInitialValueSet.m_dVNoiseDiode5GHzPWMFrequency_Hz,
                                                   m_oInitialValueSet.m_chaNoiseDiode5GHzPWMFrequencyStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzInputSource_us)
            m_pHDF5File->addNoiseDiode6_7GHzInputSource(m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzInputSource_us,
                                                  m_oInitialValueSet.m_chaVNoiseDiode6_7GHzInputSource,
                                                  m_oInitialValueSet.m_chaNoiseDiode6_7GHzInputSourceStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzLevel_us)
            m_pHDF5File->addNoiseDiode6_7GHzLevel(m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzLevel_us,
                                             m_oInitialValueSet.m_i32VNoiseDiode6_7GHzLevel,
                                             m_oInitialValueSet.m_chaNoiseDiode6_7GHzLevelStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMMark_us)
            m_pHDF5File->addNoiseDiode6_7GHzPWMMark(m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMMark_us,
                                              m_oInitialValueSet.m_i32VNoiseDiode6_7GHzPWMMark,
                                              m_oInitialValueSet.m_chaNoiseDiode6_7GHzPWMMarkStatus);
        if (m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMFrequency_us)
            m_pHDF5File->addNoiseDiode6_7GHzPWMFrequency(m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMFrequency_us,
                                                   m_oInitialValueSet.m_dVNoiseDiode6_7GHzPWMFrequency_Hz,
                                                   m_oInitialValueSet.m_chaNoiseDiode6_7GHzPWMFrequencyStatus);
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
        if (m_oInitialValueSet.m_i64TSAccumulationLength_us)
            m_pHDF5File->addAccumulationLength(m_oInitialValueSet.m_i64TSAccumulationLength_us,
                                               m_oInitialValueSet.m_u32VAccumulationLength_frames);
        if (m_oInitialValueSet.m_i64TSCoarseChannelSelect_us)
            m_pHDF5File->addCoarseChannelSelect(m_oInitialValueSet.m_i64TSCoarseChannelSelect_us,
                                                m_oInitialValueSet.m_u32VCoarseChannelSelect);
        if (m_oInitialValueSet.m_i64TSCoarseFFTShiftMask_us)
            m_pHDF5File->addCoarseFFTShiftMask(m_oInitialValueSet.m_i64TSCoarseFFTShiftMask_us,
                                               m_oInitialValueSet.m_u32VCoarseFFTShiftMask);
        if (m_oInitialValueSet.m_i64TSDspGain_us)
            m_pHDF5File->addDspGain(m_oInitialValueSet.m_i64TSDspGain_us,
                                    m_oInitialValueSet.m_dVDspGain);
        if (m_oInitialValueSet.m_i64TSAttenuationADCChan0_us)
            m_pHDF5File->addAttenuationADCChan0(m_oInitialValueSet.m_i64TSAttenuationADCChan0_us,
                                                m_oInitialValueSet.m_dVAttenuationADCChan0_dB);
        if (m_oInitialValueSet.m_i64TSAttenuationADCChan1_us)
            m_pHDF5File->addAttenuationADCChan0(m_oInitialValueSet.m_i64TSAttenuationADCChan1_us,
                                                m_oInitialValueSet.m_dVAttenuationADCChan1_dB);

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

    cout << "cHDF5FileWriter::registerCallbackHandler(): Successfully registered straight callback handler: " << pNewHandler << endl;
}

void cHDF5FileWriter::registerCallbackHandler(boost::shared_ptr<cCallbackInterface> pNewHandler)
{
    boost::unique_lock<boost::shared_mutex> oLock(m_oCallbackHandlersMutex);

    m_vpCallbackHandlers_shared.push_back(pNewHandler);

    cout << "cHDF5FileWriter::registerCallbackHandler(): Successfully registered shared callback handler: " << pNewHandler.get() << endl;
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

void cHDF5FileWriter::acsDesiredAz_callback(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAcsDesiredAzim_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsDesiredAzim_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAcsDesiredAzim_deg = dAzimuth_deg;
        sprintf(m_oInitialValueSet.m_chaAcsDesiredAzimStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAcsDesiredAz(i64Timestamp_us, dAzimuth_deg, strStatus);
}

void cHDF5FileWriter::acsDesiredEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAcsDesiredElev_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAcsDesiredElev_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVAcsDesiredElev_deg = dElevation_deg;
        sprintf(m_oInitialValueSet.m_chaAcsDesiredElevStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAcsDesiredEl(i64Timestamp_us, dElevation_deg, strStatus);
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

void cHDF5FileWriter::skyDesiredAz_callback(int64_t i64Timestamp_us,double dAzimuth_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSSkyDesiredAzim_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSSkyDesiredAzim_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVSkyDesiredAzim_deg = dAzimuth_deg;
        sprintf(m_oInitialValueSet.m_chaSkyDesiredAzimStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addSkyDesiredAz(i64Timestamp_us, dAzimuth_deg, strStatus);
}

void cHDF5FileWriter::skyDesiredEl_callback(int64_t i64Timestamp_us, double dElevation_deg, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSSkyDesiredElev_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSSkyDesiredElev_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVSkyDesiredElev_deg = dElevation_deg;
        sprintf(m_oInitialValueSet.m_chaSkyDesiredElevStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addSkyDesiredEl(i64Timestamp_us, dElevation_deg, strStatus);
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
    if(getState() == RECORDING)
        m_pHDF5File->addAntennaStatus(i64Timestamp_us, strAntennaStatus, strStatus);
}

/*  Doesn't correspond to any current function. May be implemented in station
    controller at some point?
void cHDF5FileWriter::appliedPointingModel_callback(const string &strModelName, const vector<double> &vdPointingModelParams)
{
    if(getState() != RECORDING)
        return;

    //m_pHDF5File->setAppliedPointingModel(strModelName, vdPointingModelParams);
}
*/

void cHDF5FileWriter::antennaName_callback(const string &strAntennaName)
{
    if(getState() == RECORDING)
        m_pHDF5File->setAntennaName(strAntennaName);
}

void cHDF5FileWriter::antennaDiameter_callback(const string &strAntennaDiameter)
{
    if(getState() == RECORDING)
        m_pHDF5File->setAntennaBeamwidth(strAntennaDiameter);
}

void cHDF5FileWriter::antennaBeamwidth_callback(const string &strAntennaBeamwidth)
{
    if(getState() == RECORDING)
        m_pHDF5File->setAntennaBeamwidth(strAntennaBeamwidth);
}

void cHDF5FileWriter::antennaLongitude_callback(const string &strAntennaLongitude)
{
    if(getState() == RECORDING)
        m_pHDF5File->setAntennaLongitude(strAntennaLongitude);
}

void cHDF5FileWriter::antennaLatitude_callback(const string &strAntennaLatitude)
{
    if(getState() == RECORDING)
        m_pHDF5File->setAntennaLatitude(strAntennaLatitude);
}

void cHDF5FileWriter::rNoiseDiode5GHzInputSource_callback(int64_t i64Timestamp_us, const string &strInputSource, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode5GHzInputSource_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode5GHzInputSource_us = i64Timestamp_us;
        sprintf(m_oInitialValueSet.m_chaVNoiseDiode5GHzInputSource, "%s", strInputSource.c_str());
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzInputSourceStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode5GHzInputSource(i64Timestamp_us, strInputSource, strStatus);
}

void cHDF5FileWriter::rNoiseDiode5GHzLevel_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeLevel, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode5GHzLevel_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode5GHzLevel_us = i64Timestamp_us;
        m_oInitialValueSet.m_i32VNoiseDiode5GHzLevel = i32NoiseDiodeLevel;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzLevelStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode5GHzLevel(i64Timestamp_us, i32NoiseDiodeLevel, strStatus);
}

void cHDF5FileWriter::rNoiseDiode5GHzPWMMark_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodePWMMark, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMMark_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMMark_us = i64Timestamp_us;
        m_oInitialValueSet.m_i32VNoiseDiode5GHzPWMMark = i32NoiseDiodePWMMark;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzPWMMarkStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode5GHzPWMMark(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
}

void cHDF5FileWriter::rNoiseDiode5GHzPWMFrequency_callback(int64_t i64Timestamp_us, double dNoiseDiodePWMFrequency, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMFrequency_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode5GHzPWMFrequency_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVNoiseDiode5GHzPWMFrequency_Hz = dNoiseDiodePWMFrequency;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode5GHzPWMFrequencyStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode5GHzPWMFrequency(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
}

void cHDF5FileWriter::rNoiseDiode6_7GHzInputSource_callback(int64_t i64Timestamp_us, const string &strInputSource, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzInputSource_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzInputSource_us = i64Timestamp_us;
        sprintf(m_oInitialValueSet.m_chaVNoiseDiode6_7GHzInputSource, "%s", strInputSource.c_str());
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzInputSourceStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode6_7GHzInputSource(i64Timestamp_us, strInputSource, strStatus);
}

void cHDF5FileWriter::rNoiseDiode6_7GHzLevel_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodeLevel, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzLevel_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzLevel_us = i64Timestamp_us;
        m_oInitialValueSet.m_i32VNoiseDiode6_7GHzLevel = i32NoiseDiodeLevel;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzLevelStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode6_7GHzLevel(i64Timestamp_us, i32NoiseDiodeLevel, strStatus);
}

void cHDF5FileWriter::rNoiseDiode6_7GHzPWMMark_callback(int64_t i64Timestamp_us, int32_t i32NoiseDiodePWMMark, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMMark_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMMark_us = i64Timestamp_us;
        m_oInitialValueSet.m_i32VNoiseDiode6_7GHzPWMMark = i32NoiseDiodePWMMark;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzPWMMarkStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode6_7GHzPWMMark(i64Timestamp_us, i32NoiseDiodePWMMark, strStatus);
}

void cHDF5FileWriter::rNoiseDiode6_7GHzPWMFrequency_callback(int64_t i64Timestamp_us, double dNoiseDiodePWMFrequency, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMFrequency_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSNoiseDiode6_7GHzPWMFrequency_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVNoiseDiode6_7GHzPWMFrequency_Hz = dNoiseDiodePWMFrequency;
        sprintf(m_oInitialValueSet.m_chaNoiseDiode6_7GHzPWMFrequencyStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addNoiseDiode6_7GHzPWMFrequency(i64Timestamp_us, dNoiseDiodePWMFrequency, strStatus);
}

void cHDF5FileWriter::sourceSelection_callback(int64_t i64Timestamp_us, const string &strSourceName, const string &strStatus)
{
    if(getState() == RECORDING)
        m_pHDF5File->addSourceSelection(i64Timestamp_us, strSourceName, strStatus);
}

void cHDF5FileWriter::bandSelectLcp_callback(int64_t i64Timestamp_us, bool bFrequencySelectLcp, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSBandSelectLcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSBandSelectLcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_chVBandSelectLcp = bFrequencySelectLcp;
        sprintf(m_oInitialValueSet.m_chaBandSelectLcpStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addBandSelectLcp(i64Timestamp_us, bFrequencySelectLcp, strStatus);
}

void cHDF5FileWriter::bandSelectRcp_callback(int64_t i64Timestamp_us, bool bFrequencySelectRcp, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSBandSelectRcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSBandSelectRcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_chVBandSelectRcp = bFrequencySelectRcp;
        sprintf(m_oInitialValueSet.m_chaBandSelectRcpStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addBandSelectRcp(i64Timestamp_us, bFrequencySelectRcp, strStatus);
}

void cHDF5FileWriter::frequencySky5GHz_callback(int64_t i64Timestamp_us, double dFrequencySky5GHz, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverSkyFreq5GHz_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverSkyFreq5GHz_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverSkyFreq5GHz_Hz = dFrequencySky5GHz;
        sprintf(m_oInitialValueSet.m_chaReceiverSkyFreq5GHzStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addFrequencySky5GHz(i64Timestamp_us, dFrequencySky5GHz, strStatus);
}

void cHDF5FileWriter::frequencySky6_7GHz_callback(int64_t i64Timestamp_us, double dFrequencySky6_7GHz, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverSkyFreq6_7GHz_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverSkyFreq6_7GHz_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverSkyFreq6_7GHz_Hz = dFrequencySky6_7GHz;
        sprintf(m_oInitialValueSet.m_chaReceiverSkyFreq6_7GHzStatus, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addFrequencySky6_7GHz(i64Timestamp_us, dFrequencySky6_7GHz, strStatus);
}

void cHDF5FileWriter::receiverGain5GHzLcp_callback(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverGain5GHzLcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverGain5GHzLcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverGain5GHzLcp = dGain_dB;
        sprintf(m_oInitialValueSet.m_chaReceiverGain5GHzLcp, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addReceiverGain5GHzLcp(i64Timestamp_us, dGain_dB, strStatus);
}

void cHDF5FileWriter::receiverGain5GHzRcp_callback(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverGain5GHzRcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverGain5GHzRcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverGain5GHzRcp = dGain_dB;
        sprintf(m_oInitialValueSet.m_chaReceiverGain5GHzRcp, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addReceiverGain5GHzRcp(i64Timestamp_us, dGain_dB, strStatus);
}

void cHDF5FileWriter::receiverGain6_7GHzLcp_callback(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverGain6_7GHzLcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverGain6_7GHzLcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverGain6_7GHzLcp = dGain_dB;
        sprintf(m_oInitialValueSet.m_chaReceiverGain6_7GHzLcp, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addReceiverGain6_7GHzLcp(i64Timestamp_us, dGain_dB, strStatus);
}

void cHDF5FileWriter::receiverGain6_7GHzRcp_callback(int64_t i64Timestamp_us, double dGain_dB, const string &strStatus)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSReceiverGain6_7GHzRcp_us)
    {
        boost::unique_lock<boost::shared_mutex>  oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSReceiverGain6_7GHzRcp_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVReceiverGain6_7GHzRcp = dGain_dB;
        sprintf(m_oInitialValueSet.m_chaReceiverGain6_7GHzRcp, "%s", strStatus.c_str());
    }
    if(getState() == RECORDING)
        m_pHDF5File->addReceiverGain6_7GHzRcp(i64Timestamp_us, dGain_dB, strStatus);
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
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSAccumulationLength_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSAccumulationLength_us = i64Timestamp_us;
        m_oInitialValueSet.m_u32VAccumulationLength_frames = u32NSamples;
    }
    if(getState() == RECORDING)
        m_pHDF5File->addAccumulationLength(i64Timestamp_us, u32NSamples);
}

void cHDF5FileWriter::coarseChannelSelect_callback(int64_t i64Timestamp_us, uint32_t u32ChannelNo)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSCoarseChannelSelect_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSCoarseChannelSelect_us = i64Timestamp_us;
        m_oInitialValueSet.m_u32VCoarseChannelSelect = u32ChannelNo;
    }
    if(getState() == RECORDING)
        m_pHDF5File->addCoarseChannelSelect(i64Timestamp_us, u32ChannelNo);
}

void cHDF5FileWriter::frequencyFs_callback(double dFrequencyFs_Hz)
{
    if(getState() == RECORDING)
        m_pHDF5File->setFrequencyFs(dFrequencyFs_Hz);
}

void cHDF5FileWriter::sizeOfCoarseFFT_callback(uint32_t u32SizeOfCoarseFFT_nSamp)
{
    if(getState() == RECORDING)
        m_pHDF5File->setSizeOfCoarseFFT(u32SizeOfCoarseFFT_nSamp);
}

void cHDF5FileWriter::sizeOfFineFFT_callback(uint32_t u32SizeOfFineFFT_nSamp)
{
    if(getState() == RECORDING)
        m_pHDF5File->setSizeOfFineFFT(u32SizeOfFineFFT_nSamp);
}

void cHDF5FileWriter::coarseFFTShiftMask_callback(int64_t i64Timestamp_us, uint32_t u32ShiftMask)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSCoarseFFTShiftMask_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSCoarseChannelSelect_us = i64Timestamp_us;
        m_oInitialValueSet.m_u32VCoarseFFTShiftMask = u32ShiftMask;
    }
    if(getState() == RECORDING)
        m_pHDF5File->addCoarseFFTShiftMask(i64Timestamp_us, u32ShiftMask);
}

void cHDF5FileWriter::dspGain_callback(int64_t i64Timestamp_us, double dDspGain)
{
    if (i64Timestamp_us > m_oInitialValueSet.m_i64TSRelativeHumidity_us)
    {
        boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
        m_oInitialValueSet.m_i64TSDspGain_us = i64Timestamp_us;
        m_oInitialValueSet.m_dVDspGain = dDspGain;
    }
    if(getState() == RECORDING)
        m_pHDF5File->addDspGain(i64Timestamp_us, dDspGain);
}

void cHDF5FileWriter::attenuationADCChan0_callback(int64_t i64Timestamp_us, double dADCAttenuationChan0_dB)
{
    if(getState() == RECORDING)
        m_pHDF5File->addAttenuationADCChan0(i64Timestamp_us, dADCAttenuationChan0_dB);
}

void cHDF5FileWriter::attenuationADCChan1_callback(int64_t i64Timestamp_us, double dADCAttenuationChan1_dB)
{
    if(getState() == RECORDING)
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

