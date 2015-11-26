
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

    boost::program_options::options_description oOptions("Options");
    oOptions.add_options()
            ("help", "Print this message")
            ("la", boost::program_options::value<string>(), "Local interface address (10 GbE)")
            ("lp", boost::program_options::value<uint16_t>(), "Local port (destination port in the Roach TGbE core)")
            ("ra", boost::program_options::value<string>(), "Roach host address (10 GbE)")
            ("rp", boost::program_options::value<uint16_t>(), "Roach port (source port in the Roach TGbE core)");

    boost::program_options::variables_map oVariableMap;
    boost::program_options::parsed_options oParsed = boost::program_options::command_line_parser(iArgC, pcaArgV).options(oOptions).allow_unregistered().run();
    boost::program_options::store(oParsed, oVariableMap);
    vector<string> vstrUnreckognisedOptions = boost::program_options::collect_unrecognized(oParsed.options, boost::program_options::include_positional); //Store all unrecognised arguments in vector of strings.

    string strLocalInterface("10.0.0.4");
    uint16_t u16LocalPort = 60000;
    string strRoachInterface("10.0.0.2");
    uint16_t u16RoachPort = 60000;

    if (oVariableMap.count("help"))
    {
        cout << "Usage:" << endl;
        cout << oOptions << endl;
        cout << "Example (these are also the default values if ommited):" << endl;
        cout << "\t" << pcaArgV[0] << " --la=" << strLocalInterface << " --lp=" << u16LocalPort << " --ra=" << strRoachInterface << " --rp=" << u16RoachPort << endl;
        cout << endl;
        cout << "Note press any key during program operation for clean shutdown. Press Ctrl + C to terminate immediately." << endl;
        cout << endl;

        return 1;
    }

    if (oVariableMap.count("la"))
    {
        strLocalInterface = oVariableMap["la"].as<string>();
    }

    if (oVariableMap.count("lp"))
    {
        u16LocalPort = oVariableMap["lp"].as<uint16_t>();
    }

    if (oVariableMap.count("la"))
    {
        strRoachInterface = oVariableMap["ra"].as<string>();
    }

    if (oVariableMap.count("rp"))
    {
        u16RoachPort = oVariableMap["rp"].as<uint16_t>();
    }

    if(vstrUnreckognisedOptions.size())
    {
        cout << "Warning!!!" << endl;
        cout << "Ignoring unrecognised commandline options: ";

        for(uint32_t ui = 0; ui < vstrUnreckognisedOptions.size(); ui++)
            cout << vstrUnreckognisedOptions[ui] << " ";

        cout << endl;
        cout << "Ensure to precede each argument with \"--\" and assign value with \"=\" where relevant." << endl;
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

        cRoachAcquisitionServer oServer(strLocalInterface, u16LocalPort, strRoachInterface, u16RoachPort);

        cin.get();

        cout << endl;
        cout << "----------------------------------------" << endl;
        cout << "Shutting down RoachAcquisition server..." << endl;
        cout << endl;
    }

    cout << endl << "Exited cleanly." << endl << endl;

    return 0;
}
