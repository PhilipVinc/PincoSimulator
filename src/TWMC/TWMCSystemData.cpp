//
// Created by Filippo Vicentini on 22/12/17.
//

#include "TWMCSystemData.hpp"
#include "Base/NoisyMatrix.hpp"
#include "Base/Settings.hpp"
#include "Base/Utils/EigenUtils.hpp"
#include "easylogging++.h"

#include <algorithm>
#include <iostream>

TWMCSystemData::TWMCSystemData(const Settings* settings) {
  nx = settings->get<size_t>("nx");
  ny = settings->get<size_t>("ny");

  cellSz      = settings->get<size_t>("cellSz", 1);
  latticeName = settings->get<std::string>("lattice");
  if (latticeName == "lieb") cellSz = 3;

  nxy = nx * ny;
  PBC = settings->get<bool>("PBC", false);

  if (nx == 1 && ny == 1) {
    dimension = Dimension::D0;
  } else if (nx == 1 || ny == 1) {
    dimension = Dimension::D1;
    // if we have a 1xNy lattice convert it to a Ny x 1 lattice, because it's
    // more efficient.
    if (ny == 1) {
      ny = nx;
      nx = 1;
    }
  } else {
    dimension = Dimension::D2;
  }

  U = std::unique_ptr<NoisyMatrix>(settings->GetMatrix("U", nx, ny, cellSz));
  F = std::unique_ptr<NoisyMatrix>(settings->GetMatrix("F", nx, ny, cellSz));
  omega = std::unique_ptr<NoisyMatrix>(
      settings->GetMatrix("omega", nx, ny, cellSz));
  E = std::unique_ptr<NoisyMatrix>(settings->GetMatrix("E", nx, ny, cellSz));
  // gamma = settings->GetMatrix("gamma",  nx, ny);

  // Read the Uniform Values
  detuning  = settings->get<float_p>("detuning");
  J_AB_val  = settings->get<float_p>("J_AB");
  J_BC_val  = settings->get<float_p>("J_BC");
  J_val     = settings->get<float_p>("J");
  gamma_val = settings->get<float_p>("gamma");

  // TODO: Use complex_p template specialization and add the case of a starting
  // file distrubtion!
  /*beta_init_val = settings->get<complex_p>("beta_init");*/
  /*beta_init_sigma_val = settings->get<float_p>("beta_init_sigma");*/
  beta_init_val = std::unique_ptr<NoisyMatrix>(
      settings->GetMatrix("beta_init", nx, ny, cellSz));
  beta_init_sigma_val = std::unique_ptr<NoisyMatrix>(
      settings->GetMatrix("beta_init_sigma", nx, ny, cellSz));

  // The 2D lattice is vectorized in a nx*ny line
  gamma = InitMatrix(nx, ny, cellSz, gamma_val);
  // beta_init = InitMatrix(nx, ny, cellSz, beta_init_val);

  double OBS_n_frames = settings->get<size_t>("n_frames");
  t_start             = settings->get<float_p>("t_start");
  t_end               = settings->get<float_p>("t_end");
  frames_freq         = settings->get<double>("frames_freq");

  // Backsupport
  if (frames_freq == 0) {
    frames_freq = OBS_n_frames / (t_end - t_start);
    if (OBS_n_frames == 0) {
      std::cerr << "Must have a non-zero, integer number of frames! Exiting."
                << endl;
      exit(-1);
    }
  }

  float_p timestep = settings->get<float_p>("timestep");

  size_t n_times = 0;
  float_p t      = t_start;
  while (t <= t_end) {
    n_times++;
    t += timestep;
  }
  dt          = timestep;
  n_dt        = n_times;
  frame_steps = std::max(size_t(1), size_t(ceil(frames_freq / timestep)));
  dt_obs      = dt * frame_steps;

  nFramesTot = size_t(ceil((t_end - t_start) / dt_obs));
  t_end      = nFramesTot * dt_obs;

  n_times = 0;
  t       = t_start;
  while (t <= t_end) {
    n_times++;
    t += timestep;
  }
  n_dt = n_times;

  // Fix the dt_timestep
  if (dt_obs < dt) {
    dt_obs      = dt;
    frame_steps = 1;
  }
}

size_t TWMCSystemData::ComputeNFrames(double t_start, double t_end) const {
  // return cached result
  // LOG(INFO) << "I have " << t_startCache << " and " << t_endCache << " giving
  // " << NFramesCache; LOG(INFO) << "I have been asked" << t_start << " and "
  // << t_end; LOG(INFO) << "Condition is" << bool(t_start == t_startCache &&
  // t_end == t_endCache) ; LOG(INFO) << "Result is" << size_t(nearbyint((t_end
  // - t_start) / dt_obs));

  // if (t_start == t_startCache && t_end == t_endCache) {
  //    return NFramesCache;
  //} else {
  // Compute
  //   t_startCache = t_start;
  //   t_endCache = t_end;
  size_t NFramesCache = size_t(nearbyint((t_end - t_start) / dt_obs));
  //}
  return NFramesCache;
}

TWMCSystemData::TWMCSystemData() {}

TWMCSystemData::~TWMCSystemData() {}

vector<float_p> TWMCSystemData::GetStoredTimes() const {
  size_t frames = nFramesTot + 1;
  vector<float_p> times(frames);

  float_p t      = t_start;
  size_t i_step  = 0;
  size_t i_frame = 1;

  times[0] = t_start;

  while (t <= t_end) {
    t += dt;
    i_step++;
    if ((i_step % frame_steps == 0) && i_frame < frames) {
      times[i_frame] = t;
      i_frame++;
    }
  }
  return times;
}

vector<vector<complex_p>> TWMCSystemData::GetStoredVariableEvolution(
    std::unique_ptr<NoisyMatrix> const& mat) const {
  size_t frames = nFramesTot + 1;
  vector<vector<complex_p>> result(frames, vector<complex_p>(nx * ny * cellSz));

  float_p t      = t_start;
  size_t i_step  = 0;
  size_t i_frame = 1;

  MatrixCXd m     = mat->GetAtTime(t);
  complex_p* data = m.data();

  for (int kk = 0; kk != nx * ny * cellSz; kk++) { result[0][kk] = data[kk]; }

  while (t <= t_end) {
    t += dt;
    i_step++;

    if ((i_step % frame_steps == 0) && i_frame < frames) {
      m    = mat->GetAtTime(t);
      data = m.data();
      for (int kk = 0; kk != nx * ny * cellSz; kk++) {
        result[i_frame][kk] = data[kk];
      }
      i_frame++;
    }
  }
  return result;
}
