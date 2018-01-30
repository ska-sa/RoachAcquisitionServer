#include <iostream>
#include <inttypes.h>
#include <signal.h>

#include <boost/program_options.hpp>

#include "KATCPServer.h"

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

    uint16_t    u16ServerKATCPPort;
    string      strInterface;

    boost::program_options::options_description oOptions("Options");
    oOptions.add_options()
            ("help,h", "Print this message")
            ("katcp-port,k", boost::program_options::value<uint16_t>(&u16ServerKATCPPort)->default_value(40001), "Local TCP port to listen for client KATCP connections.")
            ("listen-interface,i", boost::program_options::value<string>(&strInterface)->default_value("0.0.0.0"), "Local interface to listen for client KATCP connections.");


    boost::program_options::variables_map oVariableMap;
    boost::program_options::parsed_options oParsed = boost::program_options::command_line_parser(iArgC, pchaArgV).options(oOptions).allow_unregistered().run();
    boost::program_options::store(oParsed, oVariableMap);
    vector<string> vstrUnrecognisedOptions = boost::program_options::collect_unrecognized(oParsed.options, boost::program_options::include_positional); //Store all unrecognised arguments in vector of strings.
    boost::program_options::notify(oVariableMap);

    boost::shared_ptr<cKATCPServer> pKATCPServer;
    pKATCPServer = boost::make_shared<cKATCPServer>(strInterface, u16ServerKATCPPort, 2);

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
    cout << "Shutting down Station Controller Simulator..." << endl;
    cout << "---------------------------------------------" << endl;
    cout << endl;

    return 0;
}
