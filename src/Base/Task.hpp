//
//  Simulation.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef Simulation_hpp
#define Simulation_hpp

#include "TaskData.hpp"
class TaskResults;

#include <stdio.h>


class Task
{
public:
    Task();
    Task(size_t id);
    ~Task();
    
    virtual void Setup(TaskData* TaskData) = 0;
    virtual TaskResults* GetResults() = 0;
    virtual void Save();
    virtual float ApproximateComputationProgress();
    
    void Execute();
    
    size_t task_id;
    unsigned int seed;

protected:
    virtual void PreCompute();
    virtual void Compute() = 0;
    virtual void PostCompute();
private:
    
};



#endif /* Simulation_hpp */
