//
// Created by Filippo Vicentini on 27/11/2017.
//

#ifndef SIMULATOR_TWMCLIEBTHREADMANAGER_HPP
#define SIMULATOR_TWMCLIEBTHREADMANAGER_HPP


#include "ThreadManager.hpp"

#include <random>
#include <stdio.h>

class Settings;
class TWLiebSimData;
class TaskData;


class TWMCLiebThreadManager : public ThreadManager
{
public:
    TWMCLiebThreadManager(const Settings* settings);

    ~TWMCLiebThreadManager();

    // Get the simulation Task Data;
    virtual TaskData* SimulationData();
    virtual TaskResults* SampleTaskResult();

protected:
    virtual Task* PrepareTask(Task* _task);
    virtual Task* PrepareTask();
    //virtual void SaveTask(Task* task);
    virtual void PostUpdate();

private:

    TWLiebSimData* sharedTaskData;
    std::mt19937 seedGenerator;
};

static ManagerFactory::Registrator<TWMCLiebThreadManager> TwmcLiebThreadReg= ManagerFactory::Registrator<TWMCLiebThreadManager>("TWMCLieb");

#endif //SIMULATOR_TWMCLIEBTHREADMANAGER_HPP
