//
//  TWMCSimulation.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCSimulation_hpp
#define TWMCSimulation_hpp

// Project Includes
#include "Task.hpp"

#include "TWMCSimData.hpp"
#include "TWMCTypes.h"
// Library Includes
#include <stdio.h>

class Settings;

class TWMCSimulation : public Task
{
    
public:
    TWMCSimulation(const Settings* settings);
    
    TWMCSimData* data;
    virtual void Setup(SimData* simData);

protected:
    virtual void Compute();
private:
    TWMC_FFTW_plans* plan;
    TWMC_Results* res;
    
    
};

#endif /* TWMCSimulation_hpp */
