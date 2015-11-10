#ifndef HDF5_FILE_WRITER_H
#define HDF5_FILE_WRITER_H

//System includes
#include <string>

#ifdef _WIN32
#include <stdint.h>

#ifndef int64_t
typedef __int64 int64_t;
#endif

#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#else
#include <inttypes.h>
#endif

//Library includes
#include <hdf5.h>
#include <boost/scoped_ptr.hpp>

//Local includes
#include "AVNAppLibs/SocketStreamers/UDPReceiver/UDPReceiver.h"
#include "AVNDataTypes/SpectrometerDataStream/SpectrometerDataStreamInterpreter.h"
#include "AVNDataTypes/SpectrometerDataStream/SpectrometerHDF5OutputFile.h"

class cHDF5FileWriter : public cSpectrometerDataStreamInterpreter::cCallbackInterface, public cUDPReceiver::cCallbackInterface
{
    //cSpectrometerDataStreamInterpreter actually implements cUDPReceiver::cCallbackInterface as well and could therefore be connected directly to the UDPReceiver to get
    //the datastream from the Roach. However in the design of the class hierachy it made more sense that this HDF5Writer be the class to interface with the UDPReceiver and
    //have an instance of cSpectrometerDataStreamInterpreter as a member. This class therefore relays the offloadData_callback() call to the UDPReceiver member by regular
    //function call to have it process the data stream. It then implements cSpectrometerDataStreamInterpreter's callback interface to get the interpretted data back before
    //writing this to HDF5 file. This behaviour is little bit redundant but it offers the most flexibility if any of these classes are to be reused elsewhere.

    //The entire class hierachy will probably need to be reconsidered in time.

public:
    cHDF5FileWriter(const std::string &strRecordingDirectory);
    ~cHDF5FileWriter();

    void                                            startRecording(const std::string &strFilenamePrefix, int64_t i64StartTime_us, int64_t i64Duration_us);
    void                                            stopRecording();
    void                                            setRecordingDirectory(const std::string &strRecordingDirectory);

    //Public thread safe accessors
    bool                                            isRecordingEnabled();

    int64_t                                         getRecordedDuration_us();

    //Re-implemented callback functions
    //Data is pushed into this function by the SpectrometerDataStreamInterpreter when a complete data frame is ready
    virtual void                                    getNextFrame_callback(const std::vector<int> &vi32Chan0, const std::vector<int> &vi32Chan1, const std::vector<int> &vi32Chan2, std::vector<int> &vi32Chan3,
                                                                          const cSpectrometerHeader &oHeader);

    virtual void                                    offloadData_callback(char* cpData, uint32_t u32Size_B);

    std::string                                     makeFilename(const std::string &strDirectory, const std::string &strPrefix, int64_t i64Timestamp_us);

    enum state
    {
        IDLE = 0,
        REQUEST_START,
        RECORDING,
        REQUEST_STOP
    };

private:

    int64_t                                         m_i64LastTimestamp_us;
    int64_t                                         m_i64FrameInterval_us;
    uint32_t                                        m_u32FrameSize_nVal;
    AVN::Spectrometer::digitiserType                m_eLastDigitiserType;
    bool                                            m_eStreamChanged;

    cSpectrometerDataStreamInterpreter              m_oDataStreamInterpreter;

    boost::shared_ptr<cSpectrometerHDF5OutputFile>  m_pHDF5File;

    boost::shared_mutex                             m_oMutex;

    //Recording parameters
    std::string                                     m_strRecordingDirectory;
    std::string                                     m_strFilenamePrefix;
    std::string                                     m_strFilename;

    int64_t                                         m_i64RequestedStartTime_us;
    int64_t                                         m_i64ActualStartTime_us;
    int64_t                                         m_i64Duration_us;
    int64_t                                         m_i64StopTime_us;

    int64_t                                         m_i64RecordedDuration_us;

    int64_t                                         m_i64LastPrintTime_us;

    //State machine
    void                                            setState(state eState);
    state                                           getState();
    state                                           m_eState;

    //Private thread safe accessors
    bool                                            rejectData();

    void                                            setRecordedDuration_us(int64_t i64Duration_us);

};

#endif //
