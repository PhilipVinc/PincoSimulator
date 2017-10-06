//
//  TWMCThermoSimulation.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCThermoSimulation_hpp
#define TWMCThermoSimulation_hpp

// Project Includes
#include "Task.hpp"
#include "TWMCTypes.h"
class Settings;
class TWMCSimData;
class TWMCThermoResults;

// Library Includes
#include <stdio.h>


class TWMCThermoSimulation : public Task
{
    
public:
    TWMCThermoSimulation(const TWMCSimData* TaskData);
    ~TWMCThermoSimulation();
    
    virtual void Setup(TaskData* TaskData);
    void Initialize(unsigned int seed, size_t resultId);
    void Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0, size_t resultId);
    inline void Initialize(unsigned int seed) {Initialize(seed,seed);};
    inline void Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0){Initialize( _seed, beta_init, t0, _seed);};
    
    virtual TaskResults* GetResults();
    virtual float ApproximateComputationProgress();
    
    TWMCThermoResults* res;
    
    enum InitialConditions {ReadFromSettings,
        FixedPoint};
    
protected:
    virtual void Compute();
    InitialConditions initialCondition;
    MatrixCXd InitialState();
private:
    TWMC_FFTW_plans* plan;
    const TWMCSimData* data;
    
    size_t nx;
    size_t ny;
    
    // Thermo stuff
    MatrixRXd x;        MatrixRXd p;
    MatrixRXd dWx;      MatrixRXd dWp;
    MatrixCXd dU_t;     MatrixCXd deltaU;
    MatrixCXd dQ_t;     MatrixCXd deltaQ;
    MatrixCXd dW_t;     MatrixCXd deltaW;
    MatrixCXd dH_dt;
    MatrixCXd dH_dx;
    MatrixCXd dH_dp;
    MatrixCXd ddH_dxx;
    MatrixCXd ddH_dpp;
    
    
    // Temp sim variables
    MatrixCXd U;
    MatrixCXd F;
    MatrixCXd omega;
    MatrixCXd noise;
    
    MatrixCXd tmpRand;
    MatrixCXd kai_t;
    MatrixCXd a_t;
    MatrixCXd k_step_linear;
    MatrixCXd real_step_linear;
    float_p fft_norm_factor;
    complex_p temp_gamma;
    
    MatrixCXd beta_t_init;
    float_p t;
};

#endif /* TWMCThermoSimulation_hpp */


