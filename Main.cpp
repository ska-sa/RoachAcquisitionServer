
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

//Local includes
#include "RoachAcquisitionServer.h"

using namespace std;

int main(int iArgC, char *pcaArgV[])
{
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

    string strLocalInterface;
    uint16_t u16LocalPort;
    string strRoachInterface;
    uint16_t u16RoachPort;
    string strClientInterface;
    uint16_t u16DataPort;
    uint16_t u16KATCPPort;

    boost::program_options::options_description oOptions("Options");
    oOptions.add_options()
            ("help,h", "Print this message")
            ("local-interface,l", boost::program_options::value<string>(&strLocalInterface)->default_value(string("10.0.0.4")), "Address of local 10 GbE interface to which the Roach is connected.")
            ("udp-destination-port,d", boost::program_options::value<uint16_t>(&u16LocalPort)->default_value(60000), "Local UDP port to receive data on (destination port in the Roach TGbE core).")
            ("roach-address,r", boost::program_options::value<string>(&strRoachInterface)->default_value(string("10.0.0.2")), "IP address of the Roach's 10 GbE port connected to this computer.")
            ("udp-source-port,s", boost::program_options::value<uint16_t>(&u16RoachPort)->default_value(60001), "UDP Port on the Roach from which to packets are sent (source port in the Roach TGbE core).")
            ("client-interface,c", boost::program_options::value<string>(&strClientInterface)->default_value(string("0.0.0.0")), "Local interface to listen for client connections (data and KATCP).")
            ("data-port,d", boost::program_options::value<uint16_t>(&u16DataPort)->default_value(60001), "Local TCP port to listen for client data connections.")
            ("katcp-port,k", boost::program_options::value<uint16_t>(&u16KATCPPort)->default_value(7147), "Local TCP port to listen for client KATCP connections.");

    boost::program_options::variables_map oVariableMap;
    boost::program_options::parsed_options oParsed = boost::program_options::command_line_parser(iArgC, pcaArgV).options(oOptions).allow_unregistered().run();
    boost::program_options::store(oParsed, oVariableMap);
    vector<string> vstrUnreckognisedOptions = boost::program_options::collect_unrecognized(oParsed.options, boost::program_options::include_positional); //Store all unrecognised arguments in vector of strings.
    boost::program_options::notify(oVariableMap);

    if (oVariableMap.count("help"))
    {
        cout << oOptions << endl;
        cout << endl;
        cout << "Note press any key during program operation for clean shutdown. Press Ctrl + C to terminate immediately." << endl;
        cout << endl;

        return 1;
    }

    if(vstrUnreckognisedOptions.size())
    {
        cout << "Warning!!!" << endl;
        cout << "Ignoring unrecognised commandline options: ";

        for(uint32_t ui = 0; ui < vstrUnreckognisedOptions.size(); ui++)
            cout << vstrUnreckognisedOptions[ui] << " ";

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

    {
        cout << "Starting Roach Aquisition Server..." << endl;

        cRoachAcquisitionServer oServer(strLocalInterface, u16LocalPort, strRoachInterface, u16RoachPort, strClientInterface, u16DataPort);
        oServer.startKATCPServer(strClientInterface, u16KATCPPort);

        cin.get();

        cout << endl;
        cout << "----------------------------------------" << endl;
        cout << "Shutting down RoachAcquisition server..." << endl;
        cout << endl;
    }

    cout << endl << "Exited cleanly." << endl << endl;

    return 0;
}
