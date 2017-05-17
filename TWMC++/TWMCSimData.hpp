//
//  TWMCSimData.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCSimData_hpp
#define TWMCSimData_hpp

// Project Includes
#include "SimData.hpp"

#include "TWMCTypes.h"
// Library Includes
#include <stdio.h>


class TWMCSimData : public SimData
{
    
public:
    MatrixCXd U;
    MatrixCXd J;
    MatrixCXd F;
    MatrixCXd gamma;
    MatrixCXd omega;
    MatrixCXd beta_init;
    
    float_p U_val;
    float_p J_val;
    float_p F_val;
    float_p gamma_val;
    float_p omega_val;
    complex_p beta_init_val;
    float_p beta_init_sigma_val;
    float_p t_start;
    float_p t_end;
    float_p dt;
    float_p dt_obs;
    
    size_t n_dt;
    size_t n_frames;
    
    size_t nx;
    size_t ny;
    size_t nxy;

    
};

#endif /* TWMCSimData_hpp */
