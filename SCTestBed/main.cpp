#include <iostream>
#include <inttypes.h>

#include <boost/program_options.hpp>

//#include "KATCPServer.h"

using namespace std;

int main(int iArgC, char *pchaArgV[])
{
    uint16_t    u16ServerKATCPPort;

    boost::program_options::options_description oOptions("Options");
    oOptions.add_options()
            ("help,h", "Print this message")
            ("katcp-port,k", boost::program_options::value<uint16_t>(&u16ServerKATCPPort)->default_value(40001), "Local TCP port to listen for client KATCP connections.");


    boost::program_options::variables_map oVariableMap;
    boost::program_options::parsed_options oParsed = boost::program_options::command_line_parser(iArgC, pchaArgV).options(oOptions).allow_unregistered().run();
    boost::program_options::store(oParsed, oVariableMap);
    vector<string> vstrUnrecognisedOptions = boost::program_options::collect_unrecognized(oParsed.options, boost::program_options::include_positional); //Store all unrecognised arguments in vector of strings.
    boost::program_options::notify(oVariableMap);



    return 0;
}
