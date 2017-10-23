//
//  TWMCThreadManager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCThreadManager_hpp
#define TWMCThreadManager_hpp

#include "ThreadManager.hpp"

#include <random>
#include <stdio.h>

class Settings;
class TWMCSimData;
class TaskData;


class TWMCThreadManager : public ThreadManager
{
public:
    TWMCThreadManager(const Settings* settings);
    
    ~TWMCThreadManager();

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

static ManagerFactory::Registrator<TWMCThreadManager> TwmcThreadReg= ManagerFactory::Registrator<TWMCThreadManager>("TWMCThread");

#endif /* TWMCThreadManager_hpp */
