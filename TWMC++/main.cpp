#include <iostream>

#include "Settings.hpp"
#include "Manager.hpp"
#include "ManagerFactory.hpp"

int main(int argc, char * argv[])
{
    Settings* settings = new Settings(argc, argv);
        
    Manager* manager = ManagerFactory::create(settings);
    
    manager->ManagerLoop();
    
    return 0;
}
