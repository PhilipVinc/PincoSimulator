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
#include "Simulation.hpp"

#include "TWMCSimulation.hpp"
#include "TWMCSimData.hpp"
#include "TWMCTypes.h"
// Library Includes
#include <stdio.h>

class TWMCSimulation : public Simulation
{
    
public:
    void Compute();

    TWMCSimData* data;

protected:
    
private:
    TWMC_FFTW_plans* plan;
    TWMC_Results* res;
    
    
};

#endif /* TWMCSimulation_hpp */
