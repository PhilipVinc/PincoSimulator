//
// Created by Filippo Vicentini on 27/11/2017.
//

#ifndef SIMULATOR_TWMCLIEBSOLVER_HPP
#define SIMULATOR_TWMCLIEBSOLVER_HPP

#include "Base/Solver.hpp"

#include "TWMC/TWMCTypes.h"

#include <stdio.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <Eigen/Sparse>
typedef Eigen::SparseMatrix<complex_p, Eigen::RowMajor> SparseCXd;
#pragma clang pop

class TWMCSystemData;
class TWMCTaskData;
class Settings;
class TWLiebSimData;
class TWMCLiebResults;

class TWMCLiebSolver : public Solver
{

public:
    TWMCLiebSolver();
    ~TWMCLiebSolver();

    virtual void Setup() final;
    virtual std::vector<TaskResults*> Compute(const std::vector<TaskData*> tasks) final;

    virtual float ApproximateComputationProgress() const final;

protected:

private:
    const TWMCSystemData* lastSharedSystemData;

    size_t nx;
    size_t ny;
    size_t cellSz = 3;

    // Temp sim variables
    MatrixCXd U;
    MatrixCXd F;
    MatrixCXd E;
    MatrixCXd delta;

    MatrixCXd tmpRand;
    MatrixCXd kai_t;
    MatrixCXd a_t;
    MatrixCXd real_step_linear;
    float_p fft_norm_factor;
    complex_p temp_gamma;

    SparseCXd coupling_mat;
    SparseCXd coupling_mat_total;

    MatrixCXd beta_t_init;
    float_p t;
    float_p t_start;
    float_p t_end;
};

static SolverFactory::Registrator<TWMCLiebSolver> TWMCBaseSolver= SolverFactory::Registrator<TWMCLiebSolver::TWMCLiebSolver>("TWMCLieb");


#endif //SIMULATOR_TWMCLIEBSOLVER_HPP
