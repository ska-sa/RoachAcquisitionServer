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

protected:
  static double    getSkyActualAzim_callback(struct katcp_dispatch *pD, struct katcp_acquire *pA);

private:

  static std::string                                      m_strListenInterface;
  static uint16_t                                         m_u16Port;
  static uint32_t                                         m_u32MaxClients;

  static boost::scoped_ptr<boost::thread> m_pKATCPThread;
  static struct katcp_dispatch            *m_pKATCPDispatch;

  static boost::shared_mutex              m_oMutex;

  static double                           m_dSkyActualAzim_deg;


};

#endif //KATCTP_SERVER_H
