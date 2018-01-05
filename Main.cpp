
//System includes
#include <iostream>

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

//Library includes
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <signal.h>

//Local includes
#include "RoachAcquisitionServer.h"

using namespace std;

volatile sig_atomic_t flag = 0;
void catchControlC(int iSig)
{
  flag = 1;
}

int main(int iArgC, char *pchaArgV[])
{
    // Register signals
    signal(SIGINT, catchControlC);

    //Banner

    cout << endl;
    cout << "   ___  ___   _   ___ _  _     _                _    _ _   _            ___                                  " << endl;
    cout << "  | _ \\/ _ \\ /_\\ / __| || |   /_\\  __ __ _ _  _(_)__(_) |_(_)___ _ _   / __| ___ _ ___ _____ _ _         " << endl;
    cout << "  |   / (_) / _ \\ (__| __ |  / _ \\/ _/ _` | || | (_-< |  _| / _ \\ ' \\  \\__ \\/ -_) '_\\ V / -_) '_|     " << endl;
    cout << "  |_|_\\\\___/_/ \\_\\___|_||_| /_/ \\_\\__\\__, |\\_,_|_/__/_|\\__|_\\___/_||_| |___/\\___|_|  \\_/\\___|_| " << endl;
    cout << "                                        |_|                                                                  " << endl;
    cout << "||-----------------------------------------------------------------------------------------------||" << endl;
    cout << endl;

    cout << "Build date: " << __DATE__ << " " << __TIME__ << endl;

    cout << endl;

    string      strTenGbELocalInterface;
    uint16_t    u16TenGbELocalPort; //Destnation port of UDP traffic from ROACH FPGA
    string      strRoachTenGbEInterface;
    uint16_t    u16RoachTenGbEPort; //Source port of UDP traffic from ROACH FPGA
    string      strServerInterface; //Server that provides ROACH sample data over TCP
    uint16_t    u16ServerDataPort;
    uint16_t    u16ServerKATCPPort;
    string      strStationControllerAddress;
    uint16_t    u16StationControllerPort;
    string      strROACHPowerPCAddress;
    uint16_t    u16ROACHTCPBorphPort;
    string      strRoachGatewareDirectory;
    string      strRecordingDir;
    uint32_t    u32MaxFileSize_MB;

    boost::program_options::options_description oOptions("Options");
    oOptions.add_options()
            ("help,h", "Print this message")
            ("local-interface,l", boost::program_options::value<string>(&strTenGbELocalInterface)->default_value(string("10.0.0.4")), "Address of local 10 GbE interface to which the Roach is connected.")
            ("udp-destination-port,d", boost::program_options::value<uint16_t>(&u16TenGbELocalPort)->default_value(60000), "Local UDP port to receive data on (destination port in the Roach TGbE core).")
            ("roach-address,r", boost::program_options::value<string>(&strRoachTenGbEInterface)->default_value(string("10.0.0.2")), "IP address of the Roach's 10 GbE port connected to this computer.")
            ("udp-source-port,s", boost::program_options::value<uint16_t>(&u16RoachTenGbEPort)->default_value(60001), "UDP Port on the Roach from which to packets are sent (source port in the Roach TGbE core).")
            ("client-interface,c", boost::program_options::value<string>(&strServerInterface)->default_value(string("0.0.0.0")), "Local interface to listen for client connections (sample data and KATCP).")
            ("data-port,d", boost::program_options::value<uint16_t>(&u16ServerDataPort)->default_value(60001), "Local TCP port to listen for client data connections.")
            ("katcp-port,k", boost::program_options::value<uint16_t>(&u16ServerKATCPPort)->default_value(7147), "Local TCP port to listen for client KATCP connections.")
            ("station-controller-address,t", boost::program_options::value<string>(&strStationControllerAddress), "Address of station controller machine, providing antenna aspect information etc.")
            ("station-controller-port,u", boost::program_options::value<uint16_t>(&u16StationControllerPort)->default_value(7147), "Port to use for connection to station controller PC.")
            ("roach-ppc-address,v", boost::program_options::value<string>(&strROACHPowerPCAddress), "Address of ROACH PowerPC 1GigE interface, providing TCPBorph access.")
            ("roach-tcpborph-port,w", boost::program_options::value<uint16_t>(&u16ROACHTCPBorphPort)->default_value(7147), "Port to use for connection to ROACH TCPBorph server.")
            ("roach-gateway-dir,g", boost::program_options::value<string>(&strRoachGatewareDirectory)->default_value(string("/home/avnuser/RoachLaunchers")), "Directory containing Roach launcher script and FPG file s.")
            ("recording-dir,p", boost::program_options::value<string>(&strRecordingDir)->default_value(string("/home/avnuser/Data/RoachAcquisition")), "Path to directory where HDF5 will be recorded.")
            ("max-file-size,q", boost::program_options::value<uint32_t>(&u32MaxFileSize_MB)->default_value(1024), "Maximum HDF5 file size in MB. A new files will be created after this limit is reached (give or take metadata size). 0 implies no limit.");


    boost::program_options::variables_map oVariableMap;
    boost::program_options::parsed_options oParsed = boost::program_options::command_line_parser(iArgC, pchaArgV).options(oOptions).allow_unregistered().run();
    boost::program_options::store(oParsed, oVariableMap);
    vector<string> vstrUnrecognisedOptions = boost::program_options::collect_unrecognized(oParsed.options, boost::program_options::include_positional); //Store all unrecognised arguments in vector of strings.
    boost::program_options::notify(oVariableMap);

    if (oVariableMap.count("help"))
    {
        cout << oOptions << endl;
        cout << endl;
        cout << "Note Control-C during program operation for clean shutdown. This may take a few seconds. Press Ctrl + C again to terminate immediately (current recordings may not be saved)." << endl;
        cout << endl;

        return 1;
    }

    if(vstrUnrecognisedOptions.size())
    {
        cout << "Warning!!!" << endl;
        cout << "Ignoring unrecognised commandline options: ";

        for(uint32_t ui = 0; ui < vstrUnrecognisedOptions.size(); ui++)
            cout << vstrUnrecognisedOptions[ui] << " ";

        cout << endl;
        cout << "Will apply default values where necessary." << endl;
        cout << endl;

        for(uint32_t ui = 5; ui > 0; ui--)
        {
            cout << "Continuing to launch AcquisitionServer in " << ui << " seconds (Press Ctrl + C to cancel)" << std::flush;
            boost::this_thread::sleep(boost::posix_time::seconds(1));
            cout << "\r";
        }
        cout << endl << endl;

    }

    //Check that recording director is valid
    boost::filesystem::path oPath(strRecordingDir);
    if(!boost::filesystem::exists(oPath))
    {
        cout << "Error specified recording directory \"" << strRecordingDir << "\" does not appear to exist." << endl;
        cout << "Either create this directory or select one that does exist with the \"--recording-dir=...\" switch." << endl;
        return 1;
    }


    {
        cout << "Starting Roach Acquisition Server..." << endl;

        cRoachAcquisitionServer oServer(strTenGbELocalInterface, u16TenGbELocalPort,
                                        strRoachTenGbEInterface, u16RoachTenGbEPort,
                                        strServerInterface, u16ServerDataPort,
                                        strRecordingDir, u32MaxFileSize_MB,
                                        strRoachGatewareDirectory);

        oServer.startKATCPServer(strServerInterface, u16ServerKATCPPort);

        //Connect TCP clients to station controller and ROACH TCPBorph server if addresses are specified
        if(oVariableMap.count("station-controller-address"))
        {
            oServer.startStationControllerKATCPClient(strStationControllerAddress, u16StationControllerPort);
            //TODO: Figure out how to make these things reconnect if disconnected.
        }

        if(oVariableMap.count("roach-ppc-address"))
        {
            oServer.startRoachKATCPClient(strROACHPowerPCAddress, u16ROACHTCPBorphPort);
        }

        //Block until control - C
        while(1)
        {
            if(flag)
            {
                cout << endl;
                cout << "Caught control-C..." << endl;
                signal(SIGINT, SIG_DFL); //Clear signal handler so that subsequent control-C kills the program
                break;
            }

            boost::this_thread::sleep(boost::posix_time::milliseconds(500));
        }

        cout << endl;
        cout << "Shutting down RoachAcquisition server..." << endl;
        cout << "----------------------------------------" << endl;
        cout << endl;
    }

    cout << endl << "Exited cleanly." << endl << endl;

    return 0;
}
