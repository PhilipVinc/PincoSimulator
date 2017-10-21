//
//  WorkerThread.hpp
//  DataElaborator
//
//  Created by Filippo Vicentini on 24/03/14.
//  Copyright (c) 2014 Filippo Vicentini. All rights reserved.
//

#ifndef WorkerThread_hpp
#define WorkerThread_hpp

#include "Task.hpp"

#include "concurrentqueue.h"

#include <atomic>
#include <chrono>
#include <stdio.h>
#include <queue>

class ThreadManager;


using namespace moodycamel;
using namespace std;


class WorkerThread
{
public:
    WorkerThread(size_t id, ThreadManager* manager);
    ~WorkerThread();
    void Terminate();
    void WorkerLoop();
    
    void AssignTask(Task* task);
    void ClearSimulation();
    
    bool IsFinished();
    
    float GetSimulationSpeed();
    float GetSimulationProgress();
    
    void StartProfiling();
    void StopProfiling();
    
    size_t id;
    
protected:
    ThreadManager* manager;
    Task* currentTask;
    
    bool gotSimulation;
    bool finished;
    bool terminate;
    
    bool profileEnabled = false;
    bool monitoringTime = false;
    chrono::high_resolution_clock::time_point startTime;
    float speed = 0;
    void ClearPlan();
};



#endif /* WorkerThread_hpp */
