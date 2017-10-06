//
//  WorkerThread.cpp
//  DataElaborator
//
//  Created by Filippo Vicentini on 24/03/14.
//  Copyright (c) 2014 Filippo Vicentini. All rights reserved.
//

#include <iostream>

#include "WorkerThread.hpp"
#include "ThreadManager.hpp"

WorkerThread::WorkerThread(size_t _id, ThreadManager* _manager)
{
    id = _id;
    manager = _manager;
    
    gotSimulation = false;
    finished = false;
    terminate = false;
}

WorkerThread::~WorkerThread()
{
    
}

void WorkerThread::ClearSimulation()
{
}

void WorkerThread::WorkerLoop()
{
    while (!terminate)
    {
        currentTask = manager->GetTask(id);
        if (currentTask != NULL)
        {
            // Profiler
            if (profileEnabled)
            {
                startTime = std::chrono::high_resolution_clock::now();
                monitoringTime = true;
            }
            currentTask->Execute();
            manager->GiveResults(id, currentTask);
            ClearSimulation();

            if (profileEnabled)
            {
                auto t = chrono::high_resolution_clock::now();
                std::chrono::duration<float> dT = t - startTime;
                float dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dT).count();
                speed = 1.0/dt_ms;
                monitoringTime = false;
                manager->ReportAverageSpeed(speed);
            }
        }
    }
    manager->ReportThreadTermination(id);
}

bool WorkerThread::IsFinished()
{
    return (!gotSimulation);
}

void WorkerThread::Terminate()
{
    terminate = true;
}


// Optimizer
float WorkerThread::GetSimulationSpeed()
{
    if (currentTask != NULL && monitoringTime)
    {
        auto t = chrono::high_resolution_clock::now();
        std::chrono::duration<float> dT = t - startTime;
        float dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dT).count();
        
        return currentTask->ApproximateComputationProgress()/dt_ms;
    }
    else
    {
        return speed;
    }
    return 0;
}

float WorkerThread::GetSimulationProgress()
{
    if (currentTask != NULL)
    {
        return currentTask->ApproximateComputationProgress();
    }
    return 0;
}

void WorkerThread::StartProfiling()
{
    profileEnabled = true;
}

void WorkerThread::StopProfiling()
{
    profileEnabled = false;
}
