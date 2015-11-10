//System includes
#include <iostream>
#include <sstream>
#include <stdlib.h>

//Library includes

//Local includes
#include "KATCPServer.h"
#include "AVNUtilLibs/Timestamp/Timestamp.h"

using namespace std;

//Define static members of cKATCPServer:
struct katcp_dispatch                   *cKATCPServer::m_pKATCPDispatch;
boost::shared_ptr<cHDF5FileWriter>      cKATCPServer::m_pFileWriter;
boost::scoped_ptr<boost::thread>        cKATCPServer::m_pKATCPThread;
std::string                             cKATCPServer::m_strListenInterface;
uint16_t                                cKATCPServer::m_u16Port;
uint32_t                                cKATCPServer::m_u32MaxClients;

cKATCPServer::cKATCPServer(const string &strInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
    startServer(strInterface, u16Port, u32MaxClients);
}

cKATCPServer::cKATCPServer()
{
}

cKATCPServer::~cKATCPServer()
{
    stopServer();
}

void cKATCPServer::serverThreadFunction()
{
    // create a state handle
    m_pKATCPDispatch = startup_katcp();
    if(!m_pKATCPDispatch)
    {
        cout << "cKATCPServer::cKATCPServer(): Error allocating KATCP server state." << endl;
    }

    /* load up build and version information */
    //Get compile time
    stringstream oSSCompileTime;
    oSSCompileTime << string(__DATE__);
    oSSCompileTime << string(" ");
    oSSCompileTime << string(__TIME__);

    //Add a version number to KATCTP server
    add_version_katcp(m_pKATCPDispatch, "RoachAcquisitionServer", 0, "0.1", &oSSCompileTime.str()[0]);

    register_katcp(m_pKATCPDispatch, "?startRecording",   "start data recording to HDF5", &cKATCPServer::startRecording_callback);
    register_katcp(m_pKATCPDispatch, "?stopRecording",   "stop data recording to HDF5", &cKATCPServer::stopRecording_callback);

    //Make a server listening interface from hostname and port string
    stringstream oSSServer;
    oSSServer << m_strListenInterface;
    oSSServer << string(":");
    oSSServer << m_u16Port;

    if(run_multi_server_katcp(m_pKATCPDispatch, m_u32MaxClients, &oSSServer.str()[0], 0) < 0)
    {
        cout << "cKATCPServer::cKATCPServer(): Error starting KATCP server." << endl;
    }
}

void cKATCPServer::startServer(const string &strInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
    cout << "cKATCPServer::startServer() Starting KATCP server" << endl;

    //Store config parameters in members
    m_strListenInterface    = strInterface;
    m_u16Port               = u16Port;
    m_u32MaxClients         = u32MaxClients;

    //Launch KATCP server in a new thread
    m_pKATCPThread.reset(new boost::thread(&cKATCPServer::serverThreadFunction));
}

void cKATCPServer::stopServer()
{
    cout << "cKATCPServer::stopServer() Stopping KATCP server..." << endl;

    exited_katcp(m_pKATCPDispatch);
    shutdown_katcp(m_pKATCPDispatch);

    m_pKATCPThread->join();
    m_pKATCPThread.reset();

    cout << "cKATCPServer::stopServer() KATCP server stopped." << endl;
}

void cKATCPServer::setFileWriter(boost::shared_ptr<cHDF5FileWriter> pFileWriter)
{
    m_pFileWriter = pFileWriter;
}

int32_t cKATCPServer::startRecording_callback(struct katcp_dispatch *pKATCPDispatch, int32_t i32Argc)
{
    cout << "cKATCPServer::startRecording_callback()" << endl;

    if(!m_pFileWriter.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "startRecording: No file recorder object set.");
        return KATCP_RESULT_FAIL;
    }

    if(i32Argc > 4)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, "startRecording: Warning %i redundant argument(s) for stop recording, ignoring.", i32Argc - 4);
    }
    string strFilePrefix("");
    int64_t i64StartTime_us = 0;
    int64_t i64Duration_us = 0;

    if(i32Argc >= 2)
    {
        try
        {
            strFilePrefix = string(arg_string_katcp(pKATCPDispatch, 1));
        }
        catch(std::logic_error &oError)
        {
            cout << "cKATCPServer::startRecording_callback() Error unable to intepret filename prefix. Ignoring" << endl;
            cout << "Error was " << oError.what() << endl;
        }
    }

    if(i32Argc >= 3)
        i64StartTime_us = strtoll(arg_string_katcp(pKATCPDispatch, 2), NULL, 10);

    if(i32Argc >= 4)
        i64Duration_us = strtoll(arg_string_katcp(pKATCPDispatch, 3), NULL, 10);


    cout << "--------------------------------------------------------------" << endl;
    cout << "cKATCPServer::startRecording_callback() Got request to record:" << endl;
    cout << "File prefix = " << strFilePrefix << endl;
    cout << "Start time  = " << i64StartTime_us << "(" << AVN::stringFromTimestamp_full(i64StartTime_us) << ")" << endl;
    cout << "Duration    = " << i64Duration_us << "(" << AVN::stringFromTimeDuration(i64Duration_us) << ")" << endl;
    cout << "--------------------------------------------------------------" << endl;

    m_pFileWriter->startRecording(strFilePrefix, i64StartTime_us, i64Duration_us);

    return KATCP_RESULT_OK;
}

int32_t cKATCPServer::stopRecording_callback(struct katcp_dispatch *pKATCPDispatch, int32_t i32Argc)
{
    cout << "cKATCPServer::stopRecording_callback()" << endl;

    if(!m_pFileWriter.get())
    {
        //No file writer set. Do nothing
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_ERROR, NULL, "stopRecording: No file recorder object set.");
        return KATCP_RESULT_FAIL;
    }

    if(i32Argc > 1)
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, "stopRecording: Warning, %i redundant argument(s) for stop recording, ignoring.", i32Argc - 1);
    }

    if(!m_pFileWriter->isRecordingEnabled())
    {
        //Redundant arguments
        log_message_katcp(pKATCPDispatch, KATCP_LEVEL_WARN, NULL, "stopRecording: Warning, server is not currently recording");
        return KATCP_RESULT_FAIL;
    }

    m_pFileWriter->stopRecording();

    return KATCP_RESULT_OK;
}
