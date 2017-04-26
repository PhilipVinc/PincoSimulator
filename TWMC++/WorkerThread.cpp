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
    gotSimulator = false;
    finished = false;
    terminate = false;
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::AssignSimulatorData(TWMC_Data *data,
                                       TWMC_Results* _result,
                                       unsigned int _seed)
{
    threadMutex.lock();
    finished = false;
    input = data;
    result = _result;
    seed = _seed;
    gotSimulator = true;
    threadMutex.unlock();
}

void WorkerThread::AssignPlan(TWMC_FFTW_plans *_plan)
{
    threadMutex.lock();
    plan = _plan;
    threadMutex.unlock();
}

void WorkerThread::ClearPlan()
{
    plan = nullptr;
}

void WorkerThread::ClearSimulator()
{
    threadMutex.lock();
    input = nullptr;
    result = nullptr;
    seed = NULL;
    gotSimulator = false;
    threadMutex.unlock();
}

void WorkerThread::WorkerLoop()
{
    while (!terminate)
    {
        if (gotSimulator && (input != nullptr) && (result != nullptr) && (plan !=nullptr))
        {
            TWMC_Evolve_Parallel(id, *input, *result, *plan, seed);
            ClearSimulator();
        }
    }
    ClearPlan();
    delete this;
}

bool WorkerThread::IsFinished() {
    return (!gotSimulator);
}

void WorkerThread::Terminate()
{
    terminate = true;
}
