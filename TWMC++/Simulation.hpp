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

class Simulation
{
public:
    Simulation(int id);
    virtual ~Simulation();
    
    virtual void Setup(SimData* simData) = 0;
    virtual void PreCompute();
    virtual void Compute() = 0;
    virtual void PostCompute();
    virtual void Save();
    
    size_t th_id;
    
    unsigned int seed;

protected:

private:
    
};



#endif /* Simulation_hpp */
