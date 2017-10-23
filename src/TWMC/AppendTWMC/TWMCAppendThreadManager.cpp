//
//  TWMCAppendThreadManager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 31/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCAppendThreadManager.hpp"

#include "Settings.hpp"
#include "TWMCSimulation.hpp"
#include "TWMCSimData.hpp"
#include "TWMCResults.hpp"

#include <iostream>

using namespace std;


TWMCAppendThreadManager::TWMCAppendThreadManager(const Settings* settings) : ThreadManager(settings)
{
    sharedTaskData = new TWMCSimData(settings);
    
    // Global Seed generator
    seedGenerator = mt19937(settings->GlobalSeed());
    nTasksLeftToEnqueue = settings->get<size_t>("n_traj");
    nTotalTasks = nTasksLeftToEnqueue;
}

TWMCAppendThreadManager::~TWMCAppendThreadManager()
{
    
}

Task* TWMCAppendThreadManager::PrepareTask(Task* _task)
{
    TWMCSimulation* sim = dynamic_cast<TWMCSimulation*>(_task);
    
    sim->Initialize(seedGenerator());
    return sim;
}

Task* TWMCAppendThreadManager::PrepareTask()
{
    TWMCSimulation* task = new TWMCSimulation(sharedTaskData);
    
    return PrepareTask(task);
}

void TWMCAppendThreadManager::PostUpdate()
{
    if (nTasksLeftToEnqueue == 0)
    {
        Terminate();
    }
}

TaskData* TWMCAppendThreadManager::SimulationData()
{
    return sharedTaskData;
}

TaskResults* TWMCAppendThreadManager::SampleTaskResult()
{
    return new TWMCResults(sharedTaskData);
}

/*
class TWMCAppendThreadManagerBuilder: public ManagerFactory::Builder {
public:
    TWMCAppendThreadManagerBuilder(): ManagerFactory::Builder( "AppendTWMCThread" ) {}
    virtual Manager* build( const Settings* settings ) {
        return new TWMCAppendThreadManager( settings );
    }
};
static TWMCAppendThreadManagerBuilder TWMCAppendBuild;
*/