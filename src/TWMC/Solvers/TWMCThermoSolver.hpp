//
// Created by Filippo Vicentini on 25/12/17.
//

#ifndef SIMULATOR_TWMCTHERMOSOLVER_HPP
#define SIMULATOR_TWMCTHERMOSOLVER_HPP

#include "Base/Solver.hpp"
#include "TWMC/TWMCTypes.h"

class TWMCSystemData;
class TWMCTaskData;

class TWMCThermoSolver : public Solver
{
public:
    TWMCThermoSolver();
    ~TWMCThermoSolver();

    virtual void Setup() final;
    virtual std::vector<std::unique_ptr<TaskResults>> Compute(const std::vector<std::unique_ptr<TaskData>>& tasks) final;

    virtual float ApproximateComputationProgress() const final;

protected:

private:
    const TWMCSystemData* lastSharedSystemData;

    // Stuf to setup
    TWMC_FFTW_plans* plan;

    size_t nx;
    size_t ny;

    // Thermo stuff
    MatrixRXd x;        MatrixRXd p;
    MatrixRXd dWx;      MatrixRXd dWp;
    //MatrixCXd dU_t;     MatrixCXd deltaU;
    //MatrixCXd dQ_t;     MatrixCXd deltaQ;
    MatrixCXd dW_t;     MatrixCXd deltaW;
    MatrixCXd dA_t;     MatrixCXd deltaA;

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

    float_p t_start;
    float_p t_end;
};

static SolverFactory::Registrator<TWMCThermoSolver> TWMCBaseSolver= SolverFactory::Registrator<class TWMCThermoSolver::TWMCThermoSolver>("TWMCThermo");


#endif //SIMULATOR_TWMCTHERMOSOLVER_HPP
