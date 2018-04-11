//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCSHAREDSYSTEMDATA_HPP
#define SIMULATOR_TWMCSHAREDSYSTEMDATA_HPP

#include "Base/TaskData.hpp"
#include "TWMCTypes.h"

#include <stdio.h>
#include <memory>
#include <string>
#include <vector>

#ifdef MPI_SUPPORT
#include <cereal/types/vector.hpp>
#include "../Libraries/eigen_cereal_serialization.hpp"
#endif

class Settings;
// class NoisyMatrix;
#include "Base/NoisyMatrix.hpp"  //TODO Remove include

using namespace std;

class TWMCSystemData {
 public:
  explicit TWMCSystemData(const Settings* settings);
  TWMCSystemData();
  ~TWMCSystemData();

  vector<float_p> GetStoredTimes() const;
  vector<vector<complex_p>> GetStoredVariableEvolution(
      std::unique_ptr<NoisyMatrix> const& mat) const;

  size_t ComputeNFrames(double t_start, double t_end) const ;

  std::string latticeName;
  size_t nx;
  size_t ny;
  size_t nxy;
  size_t cellSz;

  std::unique_ptr<NoisyMatrix> U;
  std::unique_ptr<NoisyMatrix> F;
  std::unique_ptr<NoisyMatrix> omega;
  std::unique_ptr<NoisyMatrix> E;

  std::unique_ptr<NoisyMatrix> beta_init_val;
  std::unique_ptr<NoisyMatrix> beta_init_sigma_val;

  MatrixCXd J;
  MatrixCXd gamma;

  float_p J_val;
  complex_p detuning;
  float_p J_AB_val, J_BC_val;
  float_p gamma_val;
  float_p t_start;
  float_p t_end;
  float_p dt;
  float_p dt_obs;

  double frames_freq;
  size_t nFramesTot;
  size_t n_dt;
  size_t frame_steps;

  bool PBC;
  enum Dimension { D0, D1, D2 };
  Dimension dimension;

#ifdef MPI_SUPPORT
  template <class Archive>
  void serialize(Archive& ar) {
    ar(latticeName);
    ar(nx);
    ar(ny);
    ar(nxy);
    ar(cellSz);
    ar(F);
    ar(U);
    ar(omega);
    ar(E);
    ar(J);
    ar(gamma);
    ar(beta_init_val);
    ar(beta_init_sigma_val);
    ar(J_val);
    ar(detuning);
    ar(J_AB_val);
    ar(J_BC_val);
    ar(gamma_val);
    ar(beta_init_val);
    ar(beta_init_sigma_val);
    ar(t_start);
    ar(t_end);
    ar(dt);
    ar(dt_obs);
    ar(n_dt);
    ar(frames_freq);
    ar(frame_steps);
    ar(PBC);
    ar(dimension);
  }
#endif

private:
    mutable double t_startCache = NAN;
    mutable double t_endCache = NAN;
    mutable size_t NFramesCache = 0;

};

#endif  // SIMULATOR_TWMCSHAREDSYSTEMDATA_HPP
