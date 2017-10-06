#include <iostream>
#include <csignal>

#include "Base/Settings.hpp"
#include "Base/Manager.hpp"
#include "Base/Factories/ManagerFactory.hpp"

int main(int argc, char * argv[])
{
    Settings* settings = new Settings(argc, argv);

    Manager* manager = ManagerFactory::create(settings);

    manager->ManagerLoop();

    return 0;
}

