//
//  ThreadManager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ThreadManager_hpp
#define ThreadManager_hpp


#include "WorkerThread.hpp"
#include "Manager.hpp"
#include "Task.hpp"

#include "concurrentqueue.h"

#include <stdio.h>
#include <queue>
#include <vector>
#include <thread>

class Settings;


using namespace std;


class ThreadManager : public Manager
{
public:
    ThreadManager(const Settings* settings);
    ~ThreadManager();
    
    void Setup();
    
    // Queue of tasks that are waiting to be executed by a worker.
    // Fed by Manager, Eaten by Workers
    moodycamel::ConcurrentQueue<Task*> dispatchedTasks;
    // Queue of tasks completed by workers. Fed by Workers, eaten by manager
    moodycamel::ConcurrentQueue<Task*> elaboratedTasks;
    
    // Tokens to speed up queue usage.
    moodycamel::ProducerToken dispatchToken;
    moodycamel::ConsumerToken elaboratedToken;
    
    // Take a task from dispatchedTasks to execute it (called by a worker)
    Task* GetTask(size_t th_id);
    // Return a completed task, to add it to elaboratedTasks
    void GiveResults(size_t th_id, Task* task);
    // Method called by a thread to inform the manager that he is now dead
    // and can be joined.
    void ReportThreadTermination(size_t th_id);
    
    void Terminate();
    
    // Get the simulation Task Data;
    virtual TaskData* SimulationData() = 0;
    virtual TaskResults* SampleTaskResult() = 0;

protected:
    // Pool of tasks to be reused.
    queue<Task*> voidTaskPool;
    
    size_t nTasksLeftToEnqueue = 0;
    size_t nTasksToSave = 0;
    size_t nThreads = 1;
    size_t nTasksFillCapacity = 1;
    size_t nCompletedTasks = 0;
    size_t nTotalTasks = 0;
    
    virtual Task* PrepareTask(Task* _task) = 0;
    virtual Task* PrepareTask() = 0;
    
    virtual void ManagerLoop();
    virtual void PreUpdate();
    virtual void Update();
    virtual void ProcessElaboratedTasks();
    virtual void PostUpdate();
    
private:
    size_t nextWorkerId = 0;
    vector<size_t> activeThreads;
    queue<size_t> unactivatedThreadPool;
    moodycamel::ConcurrentQueue<size_t> threadsToJoin;

    
    vector<thread> threads;
    vector<WorkerThread*> workers;
    vector<ProducerToken> workersTokens;
    
    bool terminate = false;
    
    void CreateWorker();
    void TerminateWorker(size_t i);
    void TerminateAllWorkers();
    void AssignTaskToWorker();
    void EnqueueTask(size_t nTasks = 1);
    void JoinThread(size_t th_id);
    
    // Optimizer Stuff
    void OptimizeRun();
    void EnableProfilingInWorkers();
    void DisableProfilingInWorkers();
    void ComputeAverageSpeed();
    bool profiling = true;
    size_t maxProcesses = 0;
    float averageTaskComputationTime = 0.0;
    int sleepTimeMs = 1000;
    chrono::system_clock::time_point lastOptTime;
    chrono::system_clock::time_point lastPrintTime;
    chrono::system_clock::time_point startTime;
    chrono::system_clock::duration deltaTOpt;
    chrono::system_clock::duration deltaTPrint;

public:
    void ReportAverageSpeed(float speed);

};

#endif /* ThreadManager_hpp */
