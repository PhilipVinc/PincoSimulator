//
// Created by Filippo Vicentini on 27/11/2017.
//


#include "TWMCLiebThreadManager.hpp"

#include "DataSaver.hpp"
#include "Settings.hpp"
#include "TWMCLiebResults.hpp"
#include "TWLiebSimData.hpp"
#include "TWMCLiebSimulation.hpp"


#include <iostream>


using namespace std;


TWMCLiebThreadManager::TWMCLiebThreadManager(const Settings* settings) : ThreadManager(settings)
{
    sharedTaskData = new TWLiebSimData(settings);

    // Global Seed generator
    seedGenerator = mt19937(settings->GlobalSeed());
    nTasksLeftToEnqueue = settings->get<size_t>("n_traj");
    nTotalTasks = nTasksLeftToEnqueue;

    // TODO: THIS IS SHIT
    saver->ProvideDatasetNames(SampleTaskResult()->NamesOfDatasets());

    // Save the file with the times
    vector<float_p> times = sharedTaskData->GetStoredTimes();
    saver->SaveFile("_t.dat", times);
}

TWMCLiebThreadManager::~TWMCLiebThreadManager()
{

}

Task* TWMCLiebThreadManager::PrepareTask(Task* _task)
{
    TWMCLiebSimulation* sim = dynamic_cast<TWMCLiebSimulation*>(_task);

    sim->Initialize(seedGenerator());
    return sim;
}

Task* TWMCLiebThreadManager::PrepareTask()
{
    TWMCLiebSimulation* task = new TWMCLiebSimulation(sharedTaskData);

    return PrepareTask(task);
}

void TWMCLiebThreadManager::PostUpdate()
{
    if (nTasksLeftToEnqueue == 0)
    {
        Terminate();
    }
}

TaskData* TWMCLiebThreadManager::SimulationData()
{
    return sharedTaskData;
}

TaskResults* TWMCLiebThreadManager::SampleTaskResult()
{
    return new TWMCLiebResults(sharedTaskData);
}

