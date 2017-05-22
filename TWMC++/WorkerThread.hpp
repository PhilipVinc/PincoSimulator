//
//  WorkerThread.hpp
//  DataElaborator
//
//  Created by Filippo Vicentini on 24/03/14.
//  Copyright (c) 2014 Filippo Vicentini. All rights reserved.
//

#ifndef WorkerThread_hpp
#define WorkerThread_hpp

#include <stdio.h>
#include <atomic>
#include <queue>

#include "Task.hpp"
#include "concurrentqueue.h"
class ThreadManager;

using namespace moodycamel;

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
    
    size_t id;
    
protected:
    ThreadManager* manager;
    Task* currentTask;
    
    bool gotSimulation;
    bool finished;
    bool terminate;
    
    void ClearPlan();
};



#endif /* WorkerThread_hpp */
