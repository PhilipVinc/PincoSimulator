
#include "Base/Settings.hpp"
#include "Base/Manager.hpp"
#include "Base/Factories/ManagerFactory.hpp"

#include <iostream>
#include <csignal>

using namespace std;

int main(int argc, char * argv[])
{
    Settings* settings = new Settings(argc, argv);

    Manager* manager = ManagerFactory::create(settings);

    if (!(manager == NULL))
    {
        manager->ManagerLoop();
    } else {
        cout << "Invalid Manager. Exiting..." << endl;
    }


    return 0;
}

