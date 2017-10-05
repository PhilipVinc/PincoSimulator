//
//  TWMCThreadManager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ManagerFactory.hpp"
#include "Settings.hpp"
#include "TWMCThreadManager.hpp"
#include "TWMCSimulation.hpp"
#include "TWMCSimData.hpp"
#include "TWMCResults.hpp"
#include "DataSaver.hpp"

#include <iostream>

using namespace std;


TWMCThreadManager::TWMCThreadManager(const Settings* settings) : ThreadManager(settings)
{
    sharedTaskData = new TWMCSimData(settings);
    
    // Global Seed generator
    seedGenerator = mt19937(settings->GlobalSeed());
    nTasksLeftToEnqueue = settings->get<size_t>("n_traj");
    
    // TODO: THIS IS SHIT
    saver->ProvideDatasetNames(SampleTaskResult()->NamesOfDatasets());
}

TWMCThreadManager::~TWMCThreadManager()
{
    
}

Task* TWMCThreadManager::PrepareTask(Task* _task)
{
    TWMCSimulation* sim = dynamic_cast<TWMCSimulation*>(_task);
    
    sim->Initialize(seedGenerator());
    return sim;
}

Task* TWMCThreadManager::PrepareTask()
{
    TWMCSimulation* task = new TWMCSimulation(sharedTaskData);
    
    return PrepareTask(task);
}

void TWMCThreadManager::PostUpdate()
{
    if (nTasksLeftToEnqueue == 0)
    {
        Terminate();
    }
}

TaskData* TWMCThreadManager::SimulationData()
{
    return sharedTaskData;
}

TaskResults* TWMCThreadManager::SampleTaskResult()
{
    return new TWMCResults(sharedTaskData);
}


class TWMCThreadManagerBuilder: public ManagerFactory::Builder {
public:
    TWMCThreadManagerBuilder(): ManagerFactory::Builder( "TWMCThread" ) {}
    virtual Manager* build( const Settings* settings ) {
        return new TWMCThreadManager( settings );
    }
};
static TWMCThreadManagerBuilder TWMCBuild;
