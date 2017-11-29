//
// Created by Filippo Vicentini on 27/11/2017.
//

#ifndef SIMULATOR_TWMCLIEBSIMULATION_HPP
#define SIMULATOR_TWMCLIEBSIMULATION_HPP

#include "Task.hpp"
#include "TWMCTypes.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <Eigen/Sparse>
typedef Eigen::SparseMatrix<complex_p, Eigen::RowMajor> SparseCXd;
#pragma clang pop


#include <stdio.h>

class Settings;
class TWLiebSimData;
class TWMCLiebResults;


class TWMCLiebSimulation : public Task
{

public:
    TWMCLiebSimulation(const TWLiebSimData* TaskData);
    ~TWMCLiebSimulation();

    virtual void Setup(TaskData* TaskData);
    void Initialize(unsigned int seed, size_t resultId);
    void Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0, size_t resultId);
    inline void Initialize(unsigned int seed) {Initialize(seed,seed);};
    inline void Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0){Initialize( _seed, beta_init, t0, _seed);};

    virtual TaskResults* GetResults() const;
    virtual float ApproximateComputationProgress() const;

    TWMCLiebResults* res;

    enum InitialConditions {ReadFromSettings,
        FixedPoint};

protected:
    virtual void Compute();
    InitialConditions initialCondition;
    MatrixCXd InitialState();
private:
    const TWLiebSimData* data;

    size_t nx;
    size_t ny;

    // Temp sim variables
    MatrixCXd U;
    MatrixCXd F;
    MatrixCXd delta;

    MatrixCXd tmpRand;
    MatrixCXd kai_t;
    MatrixCXd a_t;
    MatrixCXd k_step_linear;
    MatrixCXd real_step_linear;
    float_p fft_norm_factor;
    complex_p temp_gamma;

    SparseCXd coupling_mat;
    SparseCXd coupling_mat_total;

    MatrixCXd beta_t_init;
    float_p t;
};


#endif //SIMULATOR_TWMCLIEBSIMULATION_HPP
