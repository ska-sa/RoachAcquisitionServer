
//System includes

//Library include

//Local includes
#include "HDF5FileWriter.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"

using namespace std;

cHDF5FileWriter::cHDF5FileWriter() :
    m_bRecordingEnabled(false),
    m_bRejectData(true)
{
}

cHDF5FileWriter::~cHDF5FileWriter()
{
    stopRecording();
}

void cHDF5FileWriter::startRecording(const string &strDirectory, const string &strFilenamePrefix, int64_t i64StartTime_us, int64_t i64Duration_us)
{
    setRecordingEnabled(true);
    setRejectData(false)  ;
}

void cHDF5FileWriter::stopRecording()
{
    boost::shared_lock<boost::shared_mutex>  oLock(m_oMutex);
    m_bRecordingEnabled = false;
}

bool cHDF5FileWriter::isRecordingEnabled()
{
    boost::shared_lock<boost::shared_mutex>  oLock(m_oMutex);
    return m_bRecordingEnabled;
}

void cHDF5FileWriter::setRecordingEnabled(bool bRecordingEnabled)
{
    boost::shared_lock<boost::shared_mutex>  oLock(m_oMutex);
    m_bRecordingEnabled = bRecordingEnabled;
}

void cHDF5FileWriter::setRejectData(bool bRejectData)
{
    boost::shared_lock<boost::shared_mutex>  oLock(m_oMutex);
    m_bRejectData = bRejectData;
}

bool cHDF5FileWriter::rejectData()
{
    boost::shared_lock<boost::shared_mutex>  oLock(m_oMutex);
    return m_bRejectData;
}

void cHDF5FileWriter::offloadData_callback(char* cpData, uint32_t u32Size_B)
{
    //Pass data on directly to the stream interpreter
    m_oDataStreamInterpreter.offloadData_callback(cpData, u32Size_B);
}

void cHDF5FileWriter::getNextFrame_callback(const std::vector<int> &vi32Chan0, const std::vector<int> &vi32Chan1, const std::vector<int> &vi32Chan2, std::vector<int> &vi32Chan3,
                                            const cSpectrometerHeader &oHeader)
{
    {
        boost::upgrade_lock< boost::shared_mutex > oLock(m_oMutex);

        if(m_i64FrameInterval_us != oHeader.getTimestamp_us() - m_i64LastTimestamp_us)
        {
            boost::upgrade_lock< boost::shared_mutex > oLock(m_oMutex);
            m_i64FrameInterval_us = oHeader.getTimestamp_us() - m_i64LastTimestamp_us;
        }


        if(m_u32FrameSize_nVal != vi32Chan0.size())
        {
            boost::upgrade_lock< boost::shared_mutex > oLock(m_oMutex);
            m_u32FrameSize_nVal = vi32Chan0.size();
        }
    }

    if(!rejectData())
    {

    }

    m_i64LastTimestamp_us = oHeader.getTimestamp_us();

}


