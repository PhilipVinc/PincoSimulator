//
//  TWMCThermoThreadManager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ManagerFactory.hpp"
#include "Settings.hpp"
#include "TWMCThermoThreadManager.hpp"
#include "TWMCThermoSimulation.hpp"
#include "TWMCSimData.hpp"
#include "TWMCThermoResults.hpp"
#include "DataSaver.hpp"

#include <iostream>

using namespace std;


TWMCThermoThreadManager::TWMCThermoThreadManager(const Settings* settings) : ThreadManager(settings)
{
    sharedTaskData = new TWMCSimData(settings);
    
    // Global Seed generator
    seedGenerator = mt19937(settings->GlobalSeed());
    nTasksLeftToEnqueue = settings->get<size_t>("n_traj");
    nTotalTasks = nTasksLeftToEnqueue;

    saver->ProvideDatasetNames(SampleTaskResult()->NamesOfDatasets());

    // Save the file with the times
    vector<float_p> times = sharedTaskData->GetStoredTimes();
    saver->SaveFile("_times.dat", times);

    // Save the time-dependent variables
    vector<vector<float_p>> F_t = sharedTaskData->GetStoredVariableEvolution(sharedTaskData->F);
    saver->SaveFile("_F_t.dat", F_t);

}

TWMCThermoThreadManager::~TWMCThermoThreadManager()
{

}

Task* TWMCThermoThreadManager::PrepareTask(Task* _task)
{
    TWMCThermoSimulation* sim = dynamic_cast<TWMCThermoSimulation*>(_task);
    
    sim->Initialize(seedGenerator());
    return sim;
}

Task* TWMCThermoThreadManager::PrepareTask()
{
    TWMCThermoSimulation* task = new TWMCThermoSimulation(sharedTaskData);
    
    return PrepareTask(task);
}

void TWMCThermoThreadManager::PostUpdate()
{
    if (nTasksLeftToEnqueue == 0)
    {
        Terminate();
    }
}

TaskData* TWMCThermoThreadManager::SimulationData()
{
    return sharedTaskData;
}

TaskResults* TWMCThermoThreadManager::SampleTaskResult()
{
    return new TWMCThermoResults(sharedTaskData);
}


class TWMCThermoThreadManagerBuilder: public ManagerFactory::Builder {
public:
    TWMCThermoThreadManagerBuilder(): ManagerFactory::Builder( "TWMCThermoThread" ) {}
    virtual Manager* build( const Settings* settings ) {
        return new TWMCThermoThreadManager( settings );
    }
};
static TWMCThermoThreadManagerBuilder TWMCThermoBuild;
