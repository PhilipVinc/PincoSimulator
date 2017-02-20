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

#include "TWMC_structures.h"

class WorkerThread
{
public:
    WorkerThread(int id);
    ~WorkerThread();
    void Terminate();
    void WorkerLoop();
    
    void AssignSimulatorData(TWMC_Data* data, TWMC_Results* result);
    void AssignPlan(TWMC_FFTW_plans* plan);
    void ClearSimulator();
     
    bool IsFinished();
    
    int id;
    
protected:
    std::atomic<TWMC_Data*> input;
    std::atomic<TWMC_Results*> result;
    std::atomic<TWMC_FFTW_plans*> plan;
    
    std::atomic<bool> gotSimulator;
    std::atomic<bool> finished;
    std::atomic<bool> terminate;
    std::mutex threadMutex;
    
    void ClearPlan();
};



#endif /* WorkerThread_hpp */
