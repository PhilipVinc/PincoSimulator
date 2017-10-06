//
//  ThreadManager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ThreadManager.hpp"
#include "DataSaver.hpp"

#include <thread>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>

using namespace std;
using namespace moodycamel;

ThreadManager::ThreadManager(const Settings* settings) : Manager(settings), dispatchToken(dispatchedTasks), elaboratedToken(elaboratedTasks)
{
    size_t n = std::thread::hardware_concurrency();
    size_t nset = settings->get<size_t>("processes");
    nset = nset == 0? 20 : nset;
    nThreads = (n == 0 ? 20 : std::min(n, nset) );
    
    maxProcesses = settings->get<size_t>("max_processes");
    if (maxProcesses == 0)
        maxProcesses = 3*n;
}

ThreadManager::~ThreadManager()
{
    while(activeThreads.size()!= 0)
    {
        // Wait a bit
    }
}

void ThreadManager::Setup()
{
    for (size_t i = 0; i != nThreads; i++)
    {
        CreateWorker();
    }
    nTasksFillCapacity = nThreads*3;
}

void ThreadManager::EnableProfilingInWorkers()
{
    for (size_t i=0; i != nThreads; i++)
    {
        workers[i]->StartProfiling();
    }
}

void ThreadManager::DisableProfilingInWorkers()
{
    for (size_t i=0; i != nThreads; i++)
    {
        workers[i]->StopProfiling();
    }
}

void ThreadManager::ManagerLoop()
{
    Setup();
    //EnableProfilingInWorkers();
    lastOptTime = chrono::system_clock::now();
    startTime = lastOptTime;
    deltaTOpt = chrono::seconds(1); deltaTPrint = chrono::seconds(1);
    while(!terminate)
    {
        PreUpdate();
        Update();
        ProcessElaboratedTasks();
        PostUpdate();
        OptimizeRun();
    }
    while(nTasksToSave != 0)
    {
        ProcessElaboratedTasks();
        OptimizeRun();
    }
    TerminateAllWorkers();
}

void ThreadManager::OptimizeRun()
{
    auto now = chrono::system_clock::now();
    chrono::system_clock::duration dt = now-lastOptTime;
    if ( dt > deltaTOpt)
    {
        // Optimize
        
        // Print
        chrono::system_clock::duration dtP = now-lastPrintTime;
        if (dtP > deltaTPrint)
        {
            chrono::system_clock::duration deltaT = now-startTime;
            int deltaTc = chrono::duration_cast<chrono::seconds>(deltaT).count();
            
            cout << "Completed " << nCompletedTasks << "/" << nTotalTasks << ".  ";
            cout << " Time: ( " << deltaTc << "/";
            if (nCompletedTasks!= 0)
                cout <<  int(deltaTc*nTotalTasks/float(nCompletedTasks));
            else
            {
                cout <<  "***";
            }
            cout << " ) ";
            
            if (nCompletedTasks >= nThreads)
            {
                deltaTPrint = chrono::seconds(int(ceil(deltaTc*nTotalTasks/float(nCompletedTasks*100))));
            }
            cout << endl;
            lastPrintTime = now;
        }
        lastOptTime = now;
    }
}

void ThreadManager::ComputeAverageSpeed()
{
    float speed = 0; 
    for (int i =0; i != nThreads; i++)
    {
        float tmp = workers[i]->GetSimulationSpeed();
        speed += (tmp != 0 ? tmp : 0);
    }
}

void ThreadManager::PreUpdate()
{

}
void ThreadManager::PostUpdate()
{
    // Properly join unused threads
    size_t th_id;
    while (threadsToJoin.try_dequeue(th_id))
    {
        cout << "Deactivating thread #" << th_id << endl;
        JoinThread(th_id);
    }
}

void ThreadManager::Update()
{
    // Fill the queue of things to do;
    size_t nTasksInQueue = dispatchedTasks.size_approx();
    if (nTasksInQueue < nTasksFillCapacity)
    {
        EnqueueTask(min(nTasksFillCapacity-nTasksInQueue, nTasksLeftToEnqueue));
    }
}

void ThreadManager::ProcessElaboratedTasks()
{
    Task* _task;
    // Save the elaborated tasks
    while (elaboratedTasks.try_dequeue(elaboratedToken, _task))
    {
        SaveTask(_task);
        voidTaskPool.push(_task);
        
        nTasksToSave--;
        nCompletedTasks++;
    }
}

void ThreadManager::EnqueueTask(size_t nTasks)
{
    for (;nTasks != 0; nTasks--)
    {
        Task* _task;
        // If we have tasks alredy setup, reuse them
        if (voidTaskPool.size() != 0)
        {
            _task = voidTaskPool.front();
            voidTaskPool.pop();
            _task = PrepareTask(_task);
        }
        else
        {
            _task = PrepareTask();
        }
        
        if (_task != nullptr)
        {
            dispatchedTasks.enqueue(dispatchToken, _task);
            nTasksToSave++;
            nTasksLeftToEnqueue--;
        }
        else
        {
            break;
        }
    }
}

void ThreadManager::CreateWorker()
{
    size_t _id;
    if (!unactivatedThreadPool.empty())
    {
        _id  = unactivatedThreadPool.front();
        unactivatedThreadPool.pop();
    }
    else
    {
        _id = nextWorkerId;
        ++nextWorkerId;
    }
    activeThreads.push_back(_id);
    WorkerThread * worker = new WorkerThread(_id, this);
    workers.push_back( worker );
    threads.push_back(thread(&WorkerThread::WorkerLoop, worker));
    workersTokens.push_back(ProducerToken(elaboratedTasks));
}

void ThreadManager::TerminateAllWorkers()
{
    for (size_t i=0; i!=activeThreads.size(); i++)
    {
        TerminateWorker(activeThreads[i]);
    }
}

void ThreadManager::TerminateWorker(size_t i)
{
    workers[i]->Terminate();
}

void ThreadManager::ReportThreadTermination(size_t th_id)
{
    threadsToJoin.enqueue(th_id);
}

void ThreadManager::JoinThread(size_t th_id)
{
    threads[th_id].join();
    delete workers[th_id];
    unactivatedThreadPool.push(th_id);
    activeThreads.erase(std::remove(activeThreads.begin(), activeThreads.end(), th_id), activeThreads.end());
}
// Methods called from other threads
Task* ThreadManager::GetTask(size_t th_id)
{
    Task* _task;
    if (dispatchedTasks.try_dequeue_from_producer(dispatchToken, _task))
        return _task;
    else
        return NULL;
}

void  ThreadManager::GiveResults(size_t th_id, Task* task)
{
    elaboratedTasks.enqueue(workersTokens[th_id], task);
}

void ThreadManager::Terminate()
{
    terminate = true;
}

void ThreadManager::ReportAverageSpeed(float speed)
{
    return;
}
