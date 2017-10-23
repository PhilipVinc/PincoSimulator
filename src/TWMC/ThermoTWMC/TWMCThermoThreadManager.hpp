//
//  TWMCThermoThreadManager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCThermoThreadManager_hpp
#define TWMCThermoThreadManager_hpp

#include "ThreadManager.hpp"

#include <random>
#include <stdio.h>

class Settings;
class TWMCSimData;


class TWMCThermoThreadManager : public ThreadManager
{
public:
    TWMCThermoThreadManager(const Settings* settings);
    
    ~TWMCThermoThreadManager();
    
    // Get the simulation Task Data;
    virtual TaskData* SimulationData();
    virtual TaskResults* SampleTaskResult();

    
protected:
    virtual Task* PrepareTask(Task* _task);
    virtual Task* PrepareTask();
    //virtual void SaveTask(Task* task);
    virtual void PostUpdate();
    
private:
    
    TWMCSimData* sharedTaskData;
    std::mt19937 seedGenerator;
    
};

static ManagerFactory::Registrator<TWMCThermoThreadManager> TwmcThreadReg= ManagerFactory::Registrator<TWMCThermoThreadManager>("TWMCThermoThread");

#endif /* TWMCThermoThreadManager_hpp */
