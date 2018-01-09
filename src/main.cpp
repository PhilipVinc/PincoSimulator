
#include "Base/Settings.hpp"
#include "Base/Manager.hpp"

#include <iostream>
#include <csignal>

using namespace std;

int main(int argc, char * argv[])
{
    Settings* settings = new Settings(argc, argv);

    Manager* manager = ManagerFactory::makeRawNewInstance(settings->get<string>("Manager"), settings);

    if (!(manager == nullptr))
    {
        manager->ManagerLoop();
    } else {
        cout << "Invalid Manager. Exiting..." << endl;
    }

    delete manager;

    return 0;
}

