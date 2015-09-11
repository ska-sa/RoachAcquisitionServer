
//System includes
#include <iostream>

//Library includes

//Local includes
#include "RoachAquisitionServer.h"

using namespace std;

int main()
{
    cout << "| ROACH Aquisition Server |" << endl;
    cout << "|-------------------------|" << endl;

    {
        cRoachAquisitionServer oServer(string("10.0.0.3"), 60001);

        cin.get();
    }

    cout << "Exiting cleanly" << endl;

    return 0;
}

