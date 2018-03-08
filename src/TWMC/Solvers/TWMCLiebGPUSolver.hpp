//
// Created by Filippo Vicentini on 26/12/17.
//
#ifdef GPU_SUPPORT
#ifndef SIMULATOR_TWMCLIEBGPUSOLVER_HPP
#define SIMULATOR_TWMCLIEBGPUSOLVER_HPP

#include "Base/SolverGPU.hpp"

#include "TWMC/TWMCTypes.h"
#include <vexcl/vexcl.hpp>

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



class TWMCLiebGPUSolver : public SolverGPU
{

public:
    TWMCLiebGPUSolver();
    ~TWMCLiebGPUSolver();

    virtual void SetupGPU(size_t gpuId);
    virtual void SuggestBatchSize();

    virtual void Setup() final;
	virtual std::vector<std::unique_ptr<TaskResults>> Compute(const std::vector<std::unique_ptr<TaskData>>& tasks) final;

    virtual float ApproximateComputationProgress() const final;

protected:

private:
    const TWMCSystemData* lastSharedSystemData;

    // VexCL Types
    vex::Context ctx;

    MatrixCXd beta_t;
    vex::vector<complex_p> beta_t_GPU;
    MatrixCXd real_step_linear;
    vex::vector<complex_p> real_step_linear_GPU;
    MatrixCXd complexU;
    vex::vector<complex_p> complexU_GPU;
    MatrixCXd iF;
    vex::vector<complex_p> iF_GPU;
    MatrixCXd complexId;
    vex::vector<complex_p> complexId_GPU;
    SparseCXd iJ;
    vex::sparse::matrix<complex_p> iJ_GPU;
    //______END_VEXCL


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
    float_p fft_norm_factor;
    complex_p temp_gamma;

    SparseCXd coupling_mat;
    SparseCXd coupling_mat_total;

    MatrixCXd beta_t_init;
    float_p t;
    float_p t_start;
    float_p t_end;

    // Progress monitor
    size_t taskBatch = 0;
    size_t completedTasks = 0;
};

static SolverFactory::Registrator<TWMCLiebGPUSolver> TWMCBaseSolver=
        SolverFactory::Registrator<class TWMCLiebGPUSolver::TWMCLiebGPUSolver>("TWMCLieb_GPU");

static SolverGPUFactory::Registrator<TWMCLiebGPUSolver> TWMCBaseSolverGPU=
        SolverGPUFactory::Registrator<class TWMCLiebGPUSolver::TWMCLiebGPUSolver>("TWMCLieb");


#endif //SIMULATOR_TWMCLIEBGPUSOLVER_HPP
#endif //GPU_SUPPORT