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
int extract_dontcare_discrete_katcp(struct katcp_dispatch *d, struct katcp_sensor *sn)
{
  return 0;
}

int extract_discrete_katcp(struct katcp_dispatch *d, struct katcp_sensor *sn)
{
  struct katcp_acquire *a;
  struct katcp_discrete_acquire *da;
  struct katcp_discrete_sensor *ds;

  a = sn->s_acquire;

  if(sn->s_type != KATCP_SENSOR_DISCRETE){
    log_message_katcp(d, KATCP_LEVEL_ERROR, NULL, "logic problem - discrete operation applied to type %d", sn->s_type);
    return -1;
  }

  ds = sn->s_more;
  da = a->a_more;

  if(da->da_current > ds->ds_size){
    log_message_katcp(d, KATCP_LEVEL_ERROR, NULL, "extracted discrete position %u for sensor %s not in advertised range 0-%d", da->da_current, sn->s_name, ds->ds_size);
    return -1;
  }

  ds->ds_current = da->da_current;

  set_status_sensor_katcp(sn, KATCP_STATUS_NOMINAL);

  return 0;
}

// Define static members of cKATCPServer
boost::scoped_ptr<boost::thread>  cKATCPServer::m_pKATCPThread;
struct katcp_dispatch             *cKATCPServer::m_pKATCPDispatch;
boost::scoped_ptr<boost::thread>  cKATCPServer::m_pSimulatorThread;
std::string                       cKATCPServer::m_strListenInterface;
uint16_t                          cKATCPServer::m_u16Port;
uint32_t                          cKATCPServer::m_u32MaxClients;

boost::shared_mutex               cKATCPServer::m_oMutex;
bool                              cKATCPServer::m_bStopSimulation = false;

double                            cKATCPServer::m_dSkyActualAzim_deg;
double                            cKATCPServer::m_dSkyActualElev_deg;
double                            cKATCPServer::m_dSkyRequestedAzim_deg;
double                            cKATCPServer::m_dSkyRequestedElev_deg;
double                            cKATCPServer::m_dAntennaActualAzim_deg;
double                            cKATCPServer::m_dAntennaActualElev_deg;
double                            cKATCPServer::m_dAntennaRequestedAzim_deg;
double                            cKATCPServer::m_dAntennaRequestedElev_deg;

double                           cKATCPServer::m_dP1;
double                           cKATCPServer::m_dP2;
double                           cKATCPServer::m_dP3;
double                           cKATCPServer::m_dP4;
double                           cKATCPServer::m_dP5;
double                           cKATCPServer::m_dP6;
double                          cKATCPServer::m_dP7;
double                           cKATCPServer::m_dP8;
double                           cKATCPServer::m_dP9;
double                           cKATCPServer::m_dP10;
double                           cKATCPServer::m_dP11;
double                           cKATCPServer::m_dP12;
double                          cKATCPServer::m_dP13;
double                          cKATCPServer::m_dP14;
double                          cKATCPServer::m_dP15;
double                           cKATCPServer::m_dP16;
double                           cKATCPServer::m_dP17;
double                           cKATCPServer::m_dP18;
double                           cKATCPServer::m_dP19;
double                           cKATCPServer::m_dP20;
double                           cKATCPServer::m_dP21;
double                           cKATCPServer::m_dP22;
double                           cKATCPServer::m_dP23;
double                           cKATCPServer::m_dP24;
double                           cKATCPServer::m_dP25;
double                           cKATCPServer::m_dP26;
double                           cKATCPServer::m_dP27;
double                           cKATCPServer::m_dP28;
double                           cKATCPServer::m_dP29;
double                           cKATCPServer::m_dP30;

int                              cKATCPServer::m_iAntennaStatus;
struct      katcp_acquire*       cKATCPServer::m_pKAAntennaStatus;
char*                            cKATCPServer::m_achaAntennaStatusDiscreteValues[]  = {"foo", "bar", "baz", "bleh"};

double                           cKATCPServer::m_dRFCIntermediate5GHz_Hz;
double                           cKATCPServer::m_dRFCIntermediate6p7GHz_Hz;
double                           cKATCPServer::m_dFinalStage_Hz;
double                           cKATCPServer::m_dLCPAttenuation_dB;
double                           cKATCPServer::m_dRCPAttenuation_dB;
bool                             cKATCPServer::m_bLCPFreqSel;
struct katcp_acquire*            cKATCPServer::m_pKALCPFreqSel;
bool                             cKATCPServer::m_bRCPFreqSel;
struct katcp_acquire*            cKATCPServer::m_pKARCPFreqSel;

uint16_t                         cKATCPServer::m_ui16NoiseDiodeState;

double                           cKATCPServer::m_dWindSpeed_mps;
double                           cKATCPServer::m_dWindDirection_deg;
double                           cKATCPServer::m_dTemperature_degC;
double                           cKATCPServer::m_dAbsolutePressure_mbar;
double                           cKATCPServer::m_dRelativeHumidity_percent;

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

/*
int cKATCPServer::extract_dontcare_discrete_katcp(struct katcp_dispatch *d, struct katcp_sensor *sn)
{
  set_status_sensor_katcp(sn, KATCP_STATUS_NOMINAL);
  return 0;
}*/

void cKATCPServer::startServer(const std::string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients)
{
  cout << "Starting KATCP server." << endl;

  //Store config parameters in members
  m_strListenInterface    = strListenInterface;
  m_u16Port               = u16Port;
  m_u32MaxClients         = u32MaxClients;

  m_pKATCPThread.reset(new boost::thread(&cKATCPServer::serverThreadFunction));
  m_pSimulatorThread.reset(new boost::thread(&cKATCPServer::dataSimulatorThreadFunction));
}

void cKATCPServer::stopServer()
{
  cout << "Stopping KATCP server." << endl;

  {
    boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);
    m_bStopSimulation = true;
  }

  terminate_katcp(m_pKATCPDispatch, KATCP_EXIT_QUIT); //Stops server

  //Make socket connection to KATCP server to force its main loop to exit
  {
      cout << "Connect temporary socket to KATCP server to force it to evaluate shutdown request..." << endl;
      cInterruptibleBlockingTCPSocket oTempSocket(m_strListenInterface, m_u16Port);
  }

  //Now we can join the threads
  m_pSimulatorThread->join();
  m_pKATCPThread->join();

  m_pSimulatorThread.reset();
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
                                &getSkyActualAzim_callback, NULL, NULL, 0, 360, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.actual-elev"),
                                const_cast<char*>("Sky-space actual elevation."),
                                const_cast<char*>("deg"),
                                &getSkyActualElev_callback, NULL, NULL, 0, 90, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.request-azim"),
                                const_cast<char*>("Sky-space requested azimuth."),
                                const_cast<char*>("deg"),
                                &getSkyRequestedAzim_callback, NULL, NULL, 0, 360, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.request-elev"),
                                const_cast<char*>("Sky-space requested elevation."),
                                const_cast<char*>("deg"),
                                &getSkyRequestedElev_callback, NULL, NULL, 0, 90, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("acs.actual-azim"),
                                const_cast<char*>("Antenna-space actual azimuth."),
                                const_cast<char*>("deg"),
                                &getAntennaActualAzim_callback, NULL, NULL, 0, 360, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("acs.actual-elev"),
                                const_cast<char*>("Antenna-space actual elevation."),
                                const_cast<char*>("deg"),
                                &getAntennaActualElev_callback, NULL, NULL, 0, 90, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("acs.request-azim"),
                                const_cast<char*>("Antenna-space requested azimuth."),
                                const_cast<char*>("deg"),
                                &getAntennaRequestedAzim_callback, NULL, NULL, 0, 360, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("acs.request-elev"),
                                const_cast<char*>("Antenna-space requested elevation."),
                                const_cast<char*>("deg"),
                                &getAntennaRequestedElev_callback, NULL, NULL, 0, 90, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel1"),
                                const_cast<char*>("P1."),
                                const_cast<char*>(""),
                                &getP1_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel2"),
                                const_cast<char*>("P2."),
                                const_cast<char*>(""),
                                &getP2_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel3"),
                                const_cast<char*>("P3."),
                                const_cast<char*>(""),
                                &getP3_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel4"),
                                const_cast<char*>("P4."),
                                const_cast<char*>(""),
                                &getP4_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel5"),
                                const_cast<char*>("P5."),
                                const_cast<char*>(""),
                                &getP5_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel6"),
                                const_cast<char*>("P6."),
                                const_cast<char*>(""),
                                &getP6_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel7"),
                                const_cast<char*>("P7."),
                                const_cast<char*>(""),
                                &getP7_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel8"),
                                const_cast<char*>("P8."),
                                const_cast<char*>(""),
                                &getP8_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel9"),
                                const_cast<char*>("P9."),
                                const_cast<char*>(""),
                                &getP9_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel10"),
                                const_cast<char*>("P10."),
                                const_cast<char*>(""),
                                &getP10_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel11"),
                                const_cast<char*>("P11."),
                                const_cast<char*>(""),
                                &getP11_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel12"),
                                const_cast<char*>("P12."),
                                const_cast<char*>(""),
                                &getP12_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel13"),
                                const_cast<char*>("P13."),
                                const_cast<char*>(""),
                                &getP13_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel14"),
                                const_cast<char*>("P14."),
                                const_cast<char*>(""),
                                &getP14_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel15"),
                                const_cast<char*>("P15."),
                                const_cast<char*>(""),
                                &getP15_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel16"),
                                const_cast<char*>("P16."),
                                const_cast<char*>(""),
                                &getP16_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel17"),
                                const_cast<char*>("P17."),
                                const_cast<char*>(""),
                                &getP17_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel18"),
                                const_cast<char*>("P18."),
                                const_cast<char*>(""),
                                &getP18_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel19"),
                                const_cast<char*>("P19."),
                                const_cast<char*>(""),
                                &getP19_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel20"),
                                const_cast<char*>("P20."),
                                const_cast<char*>(""),
                                &getP20_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel21"),
                                const_cast<char*>("P21."),
                                const_cast<char*>(""),
                                &getP21_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel22"),
                                const_cast<char*>("P22."),
                                const_cast<char*>(""),
                                &getP22_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel23"),
                                const_cast<char*>("P23."),
                                const_cast<char*>(""),
                                &getP23_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel24"),
                                const_cast<char*>("P24."),
                                const_cast<char*>(""),
                                &getP24_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel25"),
                                const_cast<char*>("P25."),
                                const_cast<char*>(""),
                                &getP25_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel26"),
                                const_cast<char*>("P26."),
                                const_cast<char*>(""),
                                &getP26_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel27"),
                                const_cast<char*>("P27."),
                                const_cast<char*>(""),
                                &getP27_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel28"),
                                const_cast<char*>("P28."),
                                const_cast<char*>(""),
                                &getP28_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel29"),
                                const_cast<char*>("P29."),
                                const_cast<char*>(""),
                                &getP29_callback, NULL, NULL, -5, 5, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.pmodel30"),
                                const_cast<char*>("P30."),
                                const_cast<char*>(""),
                                &getP30_callback, NULL, NULL, -5, 5, NULL);

  m_pKAAntennaStatus = setup_discrete_acquire_katcp(m_pKATCPDispatch, &getAntennaStatus_callback, NULL, NULL);
  register_multi_discrete_sensor_katcp(m_pKATCPDispatch, 0,
                                      const_cast<char*>("SCM.AntennaStatus"),
                                      const_cast<char*>("Antenna Status"),
                                      const_cast<char*>(""),
                                      m_achaAntennaStatusDiscreteValues, 4,
                                      m_pKAAntennaStatus, extract_discrete_katcp, NULL);

/*  register_discrete_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("SCM.AntennaStatus"),
                                const_cast<char*>("Antenna Status"),
                                const_cast<char*>(""),
                                &getAntennaStatus_callback, NULL, NULL, m_achaAntennaStatusDiscreteValues, 4);
*/
  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("RFC.IntermediateStage_5GHz"),
                                const_cast<char*>("Intermediate stage 5 GHz LO freq"),
                                const_cast<char*>("Hz"),
                                &getRFCIntermediate5GHz_callback, NULL, NULL, 1e3, 1e9, NULL);

 register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("RFC.IntermediateStage_6_7GHz"),
                                const_cast<char*>("Intermediate stage 6.7 GHz LO freq"),
                                const_cast<char*>("Hz"),
                                &getRFCIntermediate6p7GHz_callback, NULL, NULL, 1e3, 1e9, NULL);

 register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("RFC.FinalStage"),
                                const_cast<char*>("Final stage LO freq"),
                                const_cast<char*>("Hz"),
                                &getFinalStage_callback, NULL, NULL, 1e3, 1e9, NULL);

 register_double_sensor_katcp(m_pKATCPDispatch, 0,
                               const_cast<char*>("SCM.LcpAttenuation"),
                               const_cast<char*>("Receiver chain LCP attenuation"),
                               const_cast<char*>("dB"),
                               &getLCPAttenuation_callback, NULL, NULL, 0, 31.5, NULL);

 register_double_sensor_katcp(m_pKATCPDispatch, 0,
                               const_cast<char*>("SCM.RcpAttenuation"),
                               const_cast<char*>("Receiver chain RCP attenuation"),
                               const_cast<char*>("dB"),
                               &getRCPAttenuation_callback, NULL, NULL, 0, 31.5, NULL);


                               // TODO: Figure out why these are showing errors when zero. Shouldn't be.
 m_pKALCPFreqSel = setup_boolean_acquire_katcp(m_pKATCPDispatch, &getLCPFreqSel_callback, NULL, NULL);
 register_direct_multi_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                               const_cast<char*>("RFC.LcpFreqSel"),
                               const_cast<char*>("LCP Frequency Select (0 - 5GHz, 1 - 6.7GHz)"),
                               const_cast<char*>("none"),
                               m_pKALCPFreqSel);

 m_pKARCPFreqSel = setup_boolean_acquire_katcp(m_pKATCPDispatch, &getRCPFreqSel_callback, NULL, NULL);
 register_direct_multi_boolean_sensor_katcp(m_pKATCPDispatch, 0,
                               const_cast<char*>("RFC.RcpFreqSel"),
                               const_cast<char*>("RCP Frequency Select (0 - 5GHz, 1 - 6.7GHz)"),
                               const_cast<char*>("none"),
                               m_pKARCPFreqSel);

 register_integer_sensor_katcp(m_pKATCPDispatch, 0,
                               const_cast<char*>("RFC.NoiseDiode_1"),
                               const_cast<char*>("Noise diode bitfield"),
                               const_cast<char*>(""),
                               &getNoiseDiode_callback, NULL, NULL, 0, 65535, NULL);

 register_double_sensor_katcp(m_pKATCPDispatch, 0,
                               const_cast<char*>("EMS.WindSpeed"),
                               const_cast<char*>("Wind Speed"),
                               const_cast<char*>("m/s"),
                               &getWindSpeed_callback, NULL, NULL, 0, 20, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("EMS.WindDirection"),
                                const_cast<char*>("Wind Direction"),
                                const_cast<char*>("deg"),
                                &getWindDirection_callback, NULL, NULL, 0, 20, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("EMS.Temperature"),
                                const_cast<char*>("Temperature"),
                                const_cast<char*>("degC"),
                                &getTemperature_callback, NULL, NULL, 20, 40, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("EMS.AbsolutePressure"),
                                const_cast<char*>("Air pressure"),
                                const_cast<char*>("mbar"),
                                &getAbsolutePressure_callback, NULL, NULL, 500, 1500, NULL);

  register_double_sensor_katcp(m_pKATCPDispatch, 0,
                                const_cast<char*>("EMS.RelativeHumidity"),
                                const_cast<char*>("Humidity"),
                                const_cast<char*>("percent"),
                                &getRelativeHumidity_callback, NULL, NULL, 0, 100, NULL);

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

double cKATCPServer::getSkyActualElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dSkyActualElev_deg;
}

double cKATCPServer::getSkyRequestedAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dSkyRequestedAzim_deg;
}

double cKATCPServer::getSkyRequestedElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dSkyRequestedElev_deg;
}

double cKATCPServer::getAntennaActualAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dAntennaActualAzim_deg;
}

double cKATCPServer::getAntennaActualElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dAntennaActualElev_deg;
}

double cKATCPServer::getAntennaRequestedAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dAntennaRequestedAzim_deg;
}

double cKATCPServer::getAntennaRequestedElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dAntennaRequestedElev_deg;
}

double     cKATCPServer::getP1_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP1;
}

double     cKATCPServer::getP2_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP2;
}

double     cKATCPServer::getP3_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP3;
}

double     cKATCPServer::getP4_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP4;
}

double     cKATCPServer::getP5_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP5;
}

double     cKATCPServer::getP6_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP6;
}

double     cKATCPServer::getP7_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP7;
}

double     cKATCPServer::getP8_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP8;
}

double     cKATCPServer::getP9_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP9;
}

double     cKATCPServer::getP10_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP10;
}

double     cKATCPServer::getP11_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP11;
}

double     cKATCPServer::getP12_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP12;
}

double     cKATCPServer::getP13_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP13;
}

double     cKATCPServer::getP14_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP14;
}

double     cKATCPServer::getP15_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP15;
}

double     cKATCPServer::getP16_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP16;
}

double     cKATCPServer::getP17_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP17;
}

double     cKATCPServer::getP18_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP18;
}

double     cKATCPServer::getP19_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP19;
}

double     cKATCPServer::getP20_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP20;
}

double     cKATCPServer::getP21_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP21;
}

double     cKATCPServer::getP22_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP22;
}

double     cKATCPServer::getP23_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP23;
}

double     cKATCPServer::getP24_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP24;
}

double     cKATCPServer::getP25_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP25;
}

double     cKATCPServer::getP26_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP26;
}

double     cKATCPServer::getP27_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP27;
}

double     cKATCPServer::getP28_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP28;
}

double     cKATCPServer::getP29_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP29;
}

double     cKATCPServer::getP30_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dP30;
}

int cKATCPServer::getAntennaStatus_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_iAntennaStatus;
}

double cKATCPServer::getRFCIntermediate5GHz_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dRFCIntermediate5GHz_Hz;
}

double cKATCPServer::getRFCIntermediate6p7GHz_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dRFCIntermediate6p7GHz_Hz;
}

double cKATCPServer::getFinalStage_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dFinalStage_Hz;
}

double cKATCPServer::getLCPAttenuation_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dLCPAttenuation_dB;
}

double cKATCPServer::getRCPAttenuation_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dRCPAttenuation_dB;
}

int   cKATCPServer::getLCPFreqSel_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_bLCPFreqSel;
}

int   cKATCPServer::getRCPFreqSel_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_bRCPFreqSel;
}

int cKATCPServer::getNoiseDiode_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_ui16NoiseDiodeState;
}

double cKATCPServer::getWindSpeed_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dWindSpeed_mps;
}

double cKATCPServer::getWindDirection_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dWindDirection_deg;
}

double cKATCPServer::getTemperature_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dTemperature_degC;
}

double cKATCPServer::getAbsolutePressure_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dAbsolutePressure_mbar;
}

double cKATCPServer::getRelativeHumidity_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA)
{
  boost::shared_lock<boost::shared_mutex> oLock(m_oMutex);

  return m_dRelativeHumidity_percent;
}

void cKATCPServer::dataSimulatorThreadFunction()
{
  // Random seed, commented out for the time being.
  // srand(time(NULL));

  {
    // Pointing model only really needs initial values.
    boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);

    m_dP1 = (float)((rand() % 100) - 50) / 10;
    m_dP2 = (float)((rand() % 100) - 50) / 10;
    m_dP3 = (float)((rand() % 100) - 50) / 10;
    m_dP4 = (float)((rand() % 100) - 50) / 10;
    m_dP5 = (float)((rand() % 100) - 50) / 10;
    m_dP6 = (float)((rand() % 100) - 50) / 10;
    m_dP7 = (float)((rand() % 100) - 50) / 10;
    m_dP8 = (float)((rand() % 100) - 50) / 10;
    m_dP9 = (float)((rand() % 100) - 50) / 10;
    m_dP10 = (float)((rand() % 100) - 50) / 10;
    m_dP11 = (float)((rand() % 100) - 50) / 10;
    m_dP12 = (float)((rand() % 100) - 50) / 10;
    m_dP13 = (float)((rand() % 100) - 50) / 10;
    m_dP14 = (float)((rand() % 100) - 50) / 10;
    m_dP15 = (float)((rand() % 100) - 50) / 10;
    m_dP16 = (float)((rand() % 100) - 50) / 10;
    m_dP17 = (float)((rand() % 100) - 50) / 10;
    m_dP18 = (float)((rand() % 100) - 50) / 10;
    m_dP19 = (float)((rand() % 100) - 50) / 10;
    m_dP20 = (float)((rand() % 100) - 50) / 10;
    m_dP21 = (float)((rand() % 100) - 50) / 10;
    m_dP22 = (float)((rand() % 100) - 50) / 10;
    m_dP23 = (float)((rand() % 100) - 50) / 10;
    m_dP24 = (float)((rand() % 100) - 50) / 10;
    m_dP25 = (float)((rand() % 100) - 50) / 10;
    m_dP26 = (float)((rand() % 100) - 50) / 10;
    m_dP27 = (float)((rand() % 100) - 50) / 10;
    m_dP28 = (float)((rand() % 100) - 50) / 10;
    m_dP29 = (float)((rand() % 100) - 50) / 10;
    m_dP30 = (float)((rand() % 100) - 50) / 10;
  }

  while (!m_bStopSimulation)
  {
    {
      // Lock needs to be separated from the thread pause otherwise it hogs things and the other thread can never actually do its work.
      boost::unique_lock<boost::shared_mutex> oLock(m_oMutex);

      m_dSkyActualAzim_deg += (float)((rand() % 100) - 50) / 100;
      m_dSkyActualElev_deg += (float)((rand() % 100) - 50) / 100;
      m_dSkyRequestedAzim_deg += (float)((rand() % 100) - 50) / 100;
      m_dSkyRequestedElev_deg += (float)((rand() % 100) - 50) / 100;
      m_dAntennaActualAzim_deg += (float)((rand() % 100) - 50) / 100;
      m_dAntennaActualElev_deg += (float)((rand() % 100) - 50) / 100;
      m_dAntennaRequestedAzim_deg += (float)((rand() % 100) - 50) / 100;
      m_dAntennaRequestedElev_deg += (float)((rand() % 100) - 50) / 100;

      m_dRFCIntermediate5GHz_Hz += (float)((rand() % 100) + 2000);
      m_dRFCIntermediate6p7GHz_Hz += (float)((rand() % 100) + 2000);
      m_dFinalStage_Hz += (float)((rand() % 100) + 500);
      m_dLCPAttenuation_dB += (float)((rand() % 15) + 15);
      m_dRCPAttenuation_dB += (float)((rand() % 15) + 15);
      m_bLCPFreqSel = (bool)(rand() % 2);
      m_bRCPFreqSel = (bool)(rand() % 2);

      m_ui16NoiseDiodeState = (rand() % 65536);

      m_dWindSpeed_mps += (float)((rand() % 100) - 50) / 100;
      m_dWindDirection_deg += (float)((rand() % 100) - 50) / 100;
      m_dTemperature_degC += (float)((rand() % 100) - 50) / 100;
      m_dAbsolutePressure_mbar += (float)((rand() % 100) - 50) / 100;
      m_dRelativeHumidity_percent += (float)((rand() % 100) - 50) / 100;

      m_iAntennaStatus = (rand() % 4);
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  }
  cout << "Data simulator thread exiting." << endl;
}
