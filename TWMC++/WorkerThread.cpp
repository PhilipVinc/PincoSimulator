//
//  WorkerThread.cpp
//  DataElaborator
//
//  Created by Filippo Vicentini on 24/03/14.
//  Copyright (c) 2014 Filippo Vicentini. All rights reserved.
//

#include <iostream>

#include "WorkerThread.hpp"
#include "TWMC_evolve.hpp"

WorkerThread::WorkerThread(int _id)
{
    id = _id;
    gotSimulation = false;
    finished = false;
    terminate = false;
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::AssignSimulation(Simulation* _simulation)
{
    threadMutex.lock();
    finished = false;
    simulation = _simulation;
    gotSimulation = true;
    threadMutex.unlock();
}

void WorkerThread::ClearSimulation()
{
    threadMutex.lock();
    simulation = nullptr;
    gotSimulation = false;
    threadMutex.unlock();
}

void WorkerThread::WorkerLoop()
{
    while (!terminate)
    {
        if (gotSimulation && (simulation != nullptr))
        {
            simulation->PreCompute();
            simulation->Compute();
            simulation->PostCompute();
            ClearSimulation();
        }
    }
    ClearPlan();
    delete this;
}


bool WorkerThread::IsFinished() {
    return (!gotSimulation);
}

void WorkerThread::Terminate()
{
    terminate = true;
}
