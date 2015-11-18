#ifndef KATCTP_SERVER_H
#define KATCTP_SERVER_H

//System includes
#include <string>
#include <vector>

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
extern "C" {
#include <katcp.h>
}

#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#endif

//Local includes
#include "HDF5FileWriter.h"

//This class is pure static to allow callback functions to comply with the C callback format required by the KATCP library.
//This means only a single instance of this class can exist but this should suite just about every use case.
//It seems unlikely that an application would require multiple servers. In this case the class can, however, be cloned.

class cKATCPServer : public cHDF5FileWriter::cCallbackInterface
{
public:
    cKATCPServer(const std::string &strListenInterface = std::string("0.0.0.0"), uint16_t u16Port = 7147, uint32_t u32MaxClients = 1);
    cKATCPServer();
    ~cKATCPServer();

    static void                                 setFileWriter(boost::shared_ptr<cHDF5FileWriter> pFileWriter);

    static void                                 startServer(const std::string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients);
    static void                                 stopServer();

protected:
    static void                                 serverThreadFunction();

    static struct katcp_dispatch                *m_pKATCPDispatch;

    //KATCTP callbacks
    static int32_t                              startRecording_callback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                              stopRecording_callback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);
    static int32_t                              getRecordingInfo_callback(struct katcp_dispatch *pKATCPDispatch, int32_t i32ArgC);

    static boost::shared_ptr<cHDF5FileWriter>   m_pFileWriter;

    //Threads
    static boost::scoped_ptr<boost::thread>     m_pKATCPThread;

    //Members description operation state
    static std::string                          m_strListenInterface;
    static uint16_t                             m_u16Port;
    static uint32_t                             m_u32MaxClients;

    //Notification callback interface from HDF5Writer
    void recordingStarted();
    void recordingStopped();
};

#endif // KATCTP_SERVER_H
