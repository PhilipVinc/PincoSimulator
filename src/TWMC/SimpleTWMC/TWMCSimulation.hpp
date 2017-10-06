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
#include "TWMCTypes.h"
class Settings;
class TWMCSimData;
class TWMCResults;

// Library Includes
#include <stdio.h>


class TWMCSimulation : public Task
{
    
public:
    TWMCSimulation(const TWMCSimData* TaskData);
    ~TWMCSimulation();
    
    virtual void Setup(TaskData* TaskData);
    void Initialize(unsigned int seed, size_t resultId);
    void Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0, size_t resultId);
    inline void Initialize(unsigned int seed) {Initialize(seed,seed);};
    inline void Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0){Initialize( _seed, beta_init, t0, _seed);};
    
    virtual TaskResults* GetResults();
    virtual float ApproximateComputationProgress();

    TWMCResults* res;

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
    
    // Temp sim variables
    MatrixCXd U;
    MatrixCXd F;
    MatrixCXd omega;
    MatrixCXd delta;
    
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

#endif /* TWMCSimulation_hpp */
