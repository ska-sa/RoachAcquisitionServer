
//System includes
#include <iostream>

//Library includes

//Local includes
#include "RoachAquisitionServer.h"

using namespace std;

int main()
{
    cout << endl;
    cout << "|-------------------------|" << endl;
    cout << "| ROACH Aquisition Server |" << endl;
    cout << "|-------------------------|" << endl << endl;

    {
        cRoachAquisitionServer oServer(string("10.0.0.3"), 60000, string("10.0.0.2"), 60001);

        cin.get();
    }

    cout << endl << "Exiting cleanly..." << endl << endl;

    return 0;
}
