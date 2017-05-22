//
//  ThreadManager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ThreadManager_hpp
#define ThreadManager_hpp

#include <stdio.h>
#include <thread>
#include <queue>
#include <vector>

#include "concurrentqueue.h"

#include "WorkerThread.hpp"
#include "Manager.hpp"
#include "Task.hpp"

class Settings;


using namespace std;

class ThreadManager : public Manager
{
public:
    ThreadManager(const Settings* settings);
    ~ThreadManager();
    
    void Setup();
    
    moodycamel::ConcurrentQueue<Task*> dispatchedTasks;
    moodycamel::ProducerToken dispatchToken;
    moodycamel::ConcurrentQueue<Task*> elaboratedTasks;
    moodycamel::ConsumerToken elaboratedToken;

    Task* GetTask(size_t th_id);
    void GiveResults(size_t th_id, Task* task);
    void ReportThreadTermination(size_t th_id);
    
    void Terminate();
protected:
    queue<Task*> voidTaskPool;
    
    size_t nTasksLeft = 0;
    size_t nTasksToSave = 0;
    size_t nThreads = 1;
    size_t nTasksFillCapacity = 1;
    
    virtual Task* PrepareTask(Task* _task) = 0;
    virtual Task* PrepareTask() = 0;
    virtual void SaveTask(Task* task) = 0;
    
    virtual void ManagerLoop();
    virtual void PreUpdate();
    virtual void Update();
    virtual void ProcessElaboratedTasks();
    virtual void PostUpdate();

private:
    size_t nextWorkerId = 0;
    vector<size_t> activeThreads;
    queue<size_t> unactivatedThreadPool;
    
    vector<thread> threads;
    vector<WorkerThread*> workers;
    vector<ProducerToken> workersTokens;
    
    bool terminate = false;
    
    void CreateWorker();
    void TerminateWorker(size_t i);
    void TerminateAllWorkers();
    void AssignTaskToWorker();
    void EnqueueTask(size_t nTasks = 1);
};

#endif /* ThreadManager_hpp */
