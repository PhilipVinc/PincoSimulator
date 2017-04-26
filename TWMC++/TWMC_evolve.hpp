//
//  TWMC_evolve.hpp
//  TWMC-test
//
//  Created by Filippo Vicentini on 19/11/16.
//  Copyright © 2016 Filippo Vicentini. All rights reserved.
//

#ifndef TWMC_evolve_hpp
#define TWMC_evolve_hpp

#include <stdio.h>
#include "TWMC_structures.h"

void TWMC_Evolve_Parallel(size_t th_id, TWMC_Data &dat, TWMC_Results &res, TWMC_FFTW_plans &plan, unsigned int seed);

#endif /* TWMC_evolve_hpp */
