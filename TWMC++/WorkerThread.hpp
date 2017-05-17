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
#include <mutex>

#include "Simulation.hpp"

class WorkerThread
{
public:
    WorkerThread(int id);
    ~WorkerThread();
    void Terminate();
    void WorkerLoop();
    
    void AssignSimulation(Simulation* simulation);
    void ClearSimulation();
    
    bool IsFinished();
    
    int id;
    
protected:
    Simulation* simulation;
    
    std::atomic<bool> gotSimulation;
    std::atomic<bool> finished;
    std::atomic<bool> terminate;
    std::mutex threadMutex;
    
    void ClearPlan();
};



#endif /* WorkerThread_hpp */
