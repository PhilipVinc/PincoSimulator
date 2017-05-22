//
//  Simulation.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef Simulation_hpp
#define Simulation_hpp

#include <stdio.h>

#include "SimData.hpp"

class Task
{
public:
    Task();
    Task(size_t id);
    ~Task();
    
    virtual void Setup(SimData* simData) = 0;
    virtual void Save();
    
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
