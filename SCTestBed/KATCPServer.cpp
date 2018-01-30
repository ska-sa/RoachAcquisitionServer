// System includes
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <ctime>

// Library includes
extern "C" {
  #include <katpriv.h>
}

#include <boost/thread/thread.hpp>


// Local includes
#include "KATCPServer.h"
#include "../AVNUtilLibs/Sockets/InterruptibleBlockingSockets/InterruptibleBlockingTCPSocket.h"

using namespace std;

//Extract function for boolean sensors which don't care whether or not they're true or false.
//Courtesy Marc Welz
int extract_dontcare_boolean_katcp(struct katcp_dispatch *d, struct katcp_sensor *sn)
{
  set_status_sensor_katcp(sn, KATCP_STATUS_NOMINAL);
  return 0;
}

// Define static members of cKATCPServer
boost::scoped_ptr<boost::thread>  cKATCPServer::m_pKATCPThread;
struct katcp_dispatch             *cKATCPServer::m_pKATCPDispatch;
std::string                       cKATCPServer::m_strListenInterface;
uint16_t                          cKATCPServer::m_u16Port;
uint32_t                          cKATCPServer::m_u32MaxClients;

boost::shared_mutex               cKATCPServer::m_oMutex;

double                            cKATCPServer::m_dSkyActualAzim_deg;


cKATCPServer::cKATCPServer(const string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
  cout << "Starting Station Controller virtualiser..." << endl;
  startServer(strListenInterface, u16Port, u32MaxClients);
}

cKATCPServer::~cKATCPServer()
{
  cout << "Stopping Station Controller virtualiser..." << endl;
  stopServer();
  cout << "Station Controller virtualiser stopped." << endl;
}

void cKATCPServer::startServer(const std::string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
  cout << "Starting KATCP server." << endl;

  //Store config parameters in members
  m_strListenInterface    = strListenInterface;
  m_u16Port               = u16Port;
  m_u32MaxClients         = u32MaxClients;

  m_pKATCPThread.reset(new boost::thread(&cKATCPServer::serverThreadFunction));
}

void cKATCPServer::stopServer()
{
  cout << "Stopping KATCP server." << endl;

  terminate_katcp(m_pKATCPDispatch, KATCP_EXIT_QUIT); //Stops server

  //Make socket connection to KATCP server to force its main loop to exit
  {
      cout << "Connect temporary socket to KATCP server to force it to evaluate shutdown request..." << endl;
      cInterruptibleBlockingTCPSocket oTempSocket(m_strListenInterface, m_u16Port);
  }

  //Now we can join the server thread
  m_pKATCPThread->join();
  m_pKATCPThread.reset();

  shutdown_katcp(m_pKATCPDispatch); //Cleans up memory of the dispatch struct

}

void cKATCPServer::serverThreadFunction()
{
  cout << "Server thread function." << endl;
  m_pKATCPDispatch = startup_katcp();
  if(!m_pKATCPDispatch)
  {
      cout << "Error allocating KATCP server state." << endl;
  }

  /* load up build and version information */
  //Get compile time
  stringstream oSSCompileTime;
  oSSCompileTime << string(__DATE__);
  oSSCompileTime << string(" ");
  oSSCompileTime << string(__TIME__);

  //Add a version number to KATCTP server
  add_version_katcp(m_pKATCPDispatch, const_cast<char*>("StationControllerSimulator"), 0, const_cast<char*>("0.9"), &oSSCompileTime.str()[0]);

  // Sensors go in here.
  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.actual-azim"),
                                const_cast<char*>("Sky-space actual azimuth."),
                                const_cast<char*>("deg"),
                                &getSkyActualAzim_callback, NULL, NULL, 0, 10e9, NULL);

  //Make a server listening interface from hostname and port string
  stringstream oSSServer;
  oSSServer << m_strListenInterface;
  oSSServer << string(":");
  oSSServer << m_u16Port;

  if(run_multi_server_katcp(m_pKATCPDispatch, m_u32MaxClients, &oSSServer.str()[0], 0) < 0)
  {
      cout << "Error starting KATCP server." << endl;
  }

  cout << "Exiting server thread." << endl;

}

double cKATCPServer::getSkyActualAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dSkyActualAzim_deg;
}
