#ifndef ROACH_ACQUISITION_SERVER_H
#define ROACH_ACQUISITION_SERVER_H

//System includes
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

#include <vector>

//Library include:
#ifndef Q_MOC_RUN //Qt's MOC and Boost have some issues don't let MOC process boost headers
#include <boost/shared_ptr.hpp>
#endif

//Local includes
#include "AVNAppLibs/SocketStreamers/UDPReceiver/UDPReceiver.h"
#include "TCPForwardingServer.h"
#include "HDF5FileWriter.h"
#include "KATCPServer.h"

class cRoachAcquisitionServer
{
public:
    explicit cRoachAcquisitionServer(const std::string &strLocalInterface, uint16_t u16LocalPort, const std::string &strRoachTGBEAddress, uint16_t u16RoachTGBEPort);
    ~cRoachAcquisitionServer();

    void                                                start();
    void                                                shutdown();

private:

    boost::shared_ptr<cUDPReceiver>                     m_pUDPReceiver;
    boost::shared_ptr<cTCPForwardingServer>             m_pTCPForwardingServer;
    boost::shared_ptr<cHDF5FileWriter>                  m_pHDF5FileWriter;
    boost::shared_ptr<cKATCPServer>                     m_pKATCPServer;

    std::string                                         m_strRoachTGBEAddress;
    uint16_t                                            m_u16RoachTGBEPort;

    std::string                                         m_strLocalInterface;
    uint16_t                                            m_u16LocalPort;
};

#endif //ROACH_ACQUISITION_SERVER_H
