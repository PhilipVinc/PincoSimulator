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

using namespace std;
using namespace moodycamel;

ThreadManager::ThreadManager(const Settings* settings) : Manager(settings), dispatchToken(dispatchedTasks), elaboratedToken(elaboratedTasks)
{
    size_t n = std::thread::hardware_concurrency();
    size_t nset = settings->get<size_t>("processes");
    nset = nset == 0? 20 : nset;
    nThreads = (n == 0 ? 20 : std::min(n, nset) );
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

void ThreadManager::ManagerLoop()
{
    Setup();
    while(!terminate)
    {
        PreUpdate();
        Update();
        ProcessElaboratedTasks();
        PostUpdate();
    }
    while(nTasksToSave != 0)
    {
        ProcessElaboratedTasks();
    }
    TerminateAllWorkers();
}

void ThreadManager::PreUpdate()
{
    
}
void ThreadManager::PostUpdate()
{
    
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
        nTasksToSave--;
        voidTaskPool.push(_task);
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
