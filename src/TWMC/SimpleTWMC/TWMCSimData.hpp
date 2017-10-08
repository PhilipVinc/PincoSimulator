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
#include "TaskData.hpp"
#include "TWMCTypes.h"
class Settings;
class NoisyMatrix;

// Library Includes
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class TWMCSimData : public TaskData
{
    
public:
    
    TWMCSimData(const Settings* settings);
    ~TWMCSimData();
    
    NoisyMatrix* U;
    NoisyMatrix* F;
    NoisyMatrix* omega;
    MatrixCXd J;
    MatrixCXd gamma;
    
    MatrixCXd beta_init;
    
    //float_p U_val;
    float_p J_val;
    //float_p F_val;
    float_p gamma_val;
    //float_p omega_val;
    complex_p beta_init_val;
    float_p beta_init_sigma_val;
    float_p t_start;
    float_p t_end;
    float_p dt;
    float_p dt_obs;
    
    size_t n_dt;
    size_t n_frames;
    size_t frame_steps;
    
    size_t nx;
    size_t ny;
    size_t nxy;
    
    enum Dimension { D0, D1, D2 };
    Dimension dimension;

    vector<float_p> GetStoredTimes();
    vector<vector<float_p>> GetStoredVariableEvolution(NoisyMatrix* mat);
};

#endif /* TWMCSimData_hpp */
