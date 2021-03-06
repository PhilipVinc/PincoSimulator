//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCBASESOLVER_HPP
#define SIMULATOR_TWMCBASESOLVER_HPP

#include "Base/Solver.hpp"
#include "TWMC/TWMCTypes.h"

class TWMCSystemData;
class TWMCTaskData;

class TWMCBaseSolver : public Solver {
 public:
  TWMCBaseSolver();
  ~TWMCBaseSolver();

  virtual void Setup() final;
  virtual std::vector<std::unique_ptr<TaskResults>> Compute(
      const std::vector<std::unique_ptr<TaskData>>& tasks) final;

  virtual float ApproximateComputationProgress() const final;

 protected:
 private:
  const TWMCSystemData* lastSharedSystemData = nullptr;

  // Stuf to setup
  TWMC_FFTW_plans* plan = nullptr;

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

  float_p t_start;
  float_p t_end;
};

static SolverFactory::Registrator<TWMCBaseSolver> TWMCBaseSolver =
    SolverFactory::Registrator<class TWMCBaseSolver::TWMCBaseSolver>(
        "TWMCBase");

#endif  // SIMULATOR_TWMCBASESOLVER_HPP
