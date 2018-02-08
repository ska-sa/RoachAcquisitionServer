#ifndef KATCTP_SERVER_H
#define KATCTP_SERVER_H

// System includes
#include <string>
#include <vector>
#include <inttypes.h>

//Library includes
extern "C" {
#include <katcp.h>
}

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

class cKATCPServer
{
public:
  cKATCPServer(const std::string &strListenInterface = std::string("0.0.0.0"), uint16_t u16Port = 40001, uint32_t u32MaxClients = 5);
  ~cKATCPServer();

  static void startServer(const std::string &strListenInterface, uint16_t u16Port, uint32_t u32MaxClients);
  static void stopServer();
  static void serverThreadFunction();
  static void dataSimulatorThreadFunction();

  //static int extract_dontcare_discrete_katcp(struct katcp_dispatch *d, struct katcp_sensor *sn);

protected:
  static double     getSkyActualAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getSkyActualElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getSkyRequestedAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getSkyRequestedElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getAntennaActualAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getAntennaActualElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getAntennaRequestedAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getAntennaRequestedElev_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

  static double     getP1_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP2_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP3_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP4_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP5_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP6_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP7_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP8_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP9_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP10_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP11_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP12_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP13_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP14_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP15_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP16_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP17_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP18_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP19_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP20_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP21_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP22_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP23_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP24_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP25_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP26_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP27_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP28_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP29_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double     getP30_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

  static int  getAntennaStatus_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

  static double getRFCIntermediate5GHz_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getRFCIntermediate6p7GHz_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getFinalStage_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getLCPAttenuation_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getRCPAttenuation_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static int   getLCPFreqSel_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static int   getRCPFreqSel_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

  static int   getNoiseDiode_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

  static double getWindSpeed_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getWindDirection_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getTemperature_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getAbsolutePressure_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);
  static double getRelativeHumidity_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);


private:

  static std::string                                      m_strListenInterface;
  static uint16_t                                         m_u16Port;
  static uint32_t                                         m_u32MaxClients;

  static boost::scoped_ptr<boost::thread> m_pKATCPThread;
  static struct katcp_dispatch            *m_pKATCPDispatch;
  static boost::scoped_ptr<boost::thread> m_pSimulatorThread;

  static boost::shared_mutex              m_oMutex;
  static bool                             m_bStopSimulation;

  static double                           m_dSkyActualAzim_deg;
  static double                           m_dSkyActualElev_deg;
  static double                           m_dSkyRequestedAzim_deg;
  static double                           m_dSkyRequestedElev_deg;
  static double                           m_dAntennaActualAzim_deg;
  static double                           m_dAntennaActualElev_deg;
  static double                           m_dAntennaRequestedAzim_deg;
  static double                           m_dAntennaRequestedElev_deg;

  static double                           m_dP1;
  static double                           m_dP2;
  static double                           m_dP3;
  static double                           m_dP4;
  static double                           m_dP5;
  static double                           m_dP6;
  static double                           m_dP7;
  static double                           m_dP8;
  static double                           m_dP9;
  static double                           m_dP10;
  static double                           m_dP11;
  static double                           m_dP12;
  static double                           m_dP13;
  static double                           m_dP14;
  static double                           m_dP15;
  static double                           m_dP16;
  static double                           m_dP17;
  static double                           m_dP18;
  static double                           m_dP19;
  static double                           m_dP20;
  static double                           m_dP21;
  static double                           m_dP22;
  static double                           m_dP23;
  static double                           m_dP24;
  static double                           m_dP25;
  static double                           m_dP26;
  static double                           m_dP27;
  static double                           m_dP28;
  static double                           m_dP29;
  static double                           m_dP30;

  static struct katcp_acquire*            m_pKAAntennaStatus;
  static int                            m_iAntennaStatus;
  static char*                           m_achaAntennaStatusDiscreteValues[4];

  static double                           m_dRFCIntermediate5GHz_Hz;
  static double                           m_dRFCIntermediate6p7GHz_Hz;
  static double                           m_dFinalStage_Hz;
  static double                           m_dLCPAttenuation_dB;
  static double                           m_dRCPAttenuation_dB;
  static bool                             m_bLCPFreqSel;
  static struct katcp_acquire*            m_pKALCPFreqSel;
  static bool                             m_bRCPFreqSel;
  static struct katcp_acquire*            m_pKARCPFreqSel;

  static uint16_t                         m_ui16NoiseDiodeState;

  static double                           m_dWindSpeed_mps;
  static double                           m_dWindDirection_deg;
  static double                           m_dTemperature_degC;
  static double                           m_dAbsolutePressure_mbar;
  static double                           m_dRelativeHumidity_percent;

};

#endif //KATCTP_SERVER_H
