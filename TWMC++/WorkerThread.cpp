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
            currentTask->Execute();
            manager->GiveResults(id, currentTask);
            ClearSimulation();
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
