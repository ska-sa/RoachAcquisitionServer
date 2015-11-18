
//System includes
#include <iostream>

//Library includes

//Local includes
#include "RoachAcquisitionServer.h"

using namespace std;

int main()
{
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

    {
        cout << "Starting Roach Aquisition Server..." << endl;

        cRoachAcquisitionServer oServer(string("10.0.0.4"), 60000, string("10.0.0.2"), 60001);

        cin.get();

        cout << endl;
        cout << "-----------------------------------------" << endl;
        cout << "Shutdown down RoachAcquisition server...." << endl;
    }

    cout << endl << "Exited cleanly." << endl << endl;

    return 0;
}
