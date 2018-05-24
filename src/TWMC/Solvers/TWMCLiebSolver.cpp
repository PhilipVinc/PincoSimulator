//
// Created by Filippo Vicentini on 27/11/2017.
//

#include "TWMCLiebSolver.hpp"

#include "TWMC/TWMCResults.hpp"
#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"

#include "Base/NoisyMatrix.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>

using namespace std;

// Local Utility Methods
inline void randCMat(MatrixCXd *mat, std::mt19937 &gen,
                     std::normal_distribution<> norm);
inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm);

TWMCLiebSolver::TWMCLiebSolver() { nTasksToRequest = 1; }

TWMCLiebSolver::~TWMCLiebSolver() {}

void TWMCLiebSolver::Setup() {
  const TWMCSystemData *data = lastSharedSystemData;

  nx = data->nx;
  ny = data->ny;

  if (data->dimension == TWMCSystemData::Dimension::D1 ||
      data->dimension == TWMCSystemData::Dimension::D0) {
    // Temporary variables, for now ny = 1;
    temp_gamma = sqrt(data->gamma_val * data->dt / 4.0);
    tmpRand    = MatrixCXd::Zero(data->nx, 3 * data->ny);
    kai_t      = MatrixCXd::Zero(data->nx, data->ny);
    a_t        = MatrixCXd::Zero(data->nx, 3 * data->ny);

    // Merge the three variants in a single for Delta
    delta = -data->E->GenerateNoNoise().array() + data->detuning;
    real_step_linear = (-ij * delta.array() - data->gamma_val / 2.0);

    // For U
    U = data->U->GenerateNoNoise();
    F = data->F->GenerateNoNoise();


    coupling_mat = SparseCXd(3 * ny, 3 * ny);

    std::vector<Eigen::Triplet<complex_p>> elList;
    for (int i = 0; i != data->ny; i++) {
      // Internal coupling
      // AB
      elList.push_back(
          Eigen::Triplet<complex_p>(3 * i, 3 * i + 1, data->J_AB_val));
      elList.push_back(
          Eigen::Triplet<complex_p>(3 * i + 1, 3 * i, data->J_AB_val));
      // BC
      elList.push_back(
          Eigen::Triplet<complex_p>(3 * i + 1, 3 * i + 2, data->J_BC_val));
      elList.push_back(
          Eigen::Triplet<complex_p>(3 * i + 2, 3 * i + 1, data->J_BC_val));

      // Inter-Site Coupling BC. Skip for last element
      if (i == data->ny - 1) {
        // End of chain. If there are PBC, put them. Otherwise stop.
        if (data->PBC) {
          elList.push_back(
              Eigen::Triplet<complex_p>(3 * i + 2, 0 + 1, data->J_BC_val));
          elList.push_back(
              Eigen::Triplet<complex_p>(0 + 1, 3 * i + 2, data->J_BC_val));
        }
        break;
      }

      elList.push_back(Eigen::Triplet<complex_p>(3 * i + 2, 3 * (i + 1) + 1,
                                                 data->J_BC_val));
      elList.push_back(Eigen::Triplet<complex_p>(3 * (i + 1) + 1, 3 * i + 2,
                                                 data->J_BC_val));
    }
    coupling_mat.setFromTriplets(elList.begin(), elList.end());

    for (int i = 0; i != cellSz * data->ny; i++) {
      elList.push_back(
          Eigen::Triplet<complex_p>(i, i, -real_step_linear(i) / ij));
    }
    coupling_mat_total = SparseCXd(cellSz * ny, cellSz * ny);
    coupling_mat_total.setFromTriplets(elList.begin(), elList.end());

    // cout << coupling_mat_total << endl;

  } else if (data->dimension == TWMCSystemData::Dimension::D2) {
    std::cerr << "ERROR: 2D Not Supported for LIEB " << endl;
  }

  // If we have a 1D system, then we put to 0 it's contribution of the cosinus
  // (1D/2D) code.
  double flag1DNx = (data->nx == 1) ? 0.0 : 1.0;
  double flag1DNy = (data->ny == 1) ? 0.0 : 1.0;
}

//
// The Truncated Wigner Evolution Method for a Lieb Lattice.
//
std::vector<std::unique_ptr<TaskResults>> TWMCLiebSolver::Compute(
    const std::vector<std::unique_ptr<TaskData>> &tasks) {
  // cout << "computing" <<endl;
  std::vector<std::unique_ptr<TaskResults>> allResults;
  for (size_t i = 0; i < tasks.size(); i++) {
    TWMCTaskData *task = static_cast<TWMCTaskData *>(tasks[i].get());

    // Check if the system is always the same
    if (task->systemData.get() != lastSharedSystemData) {
      lastSharedSystemData = task->systemData.get();
      Setup();
    }
    const TWMCSystemData *data = lastSharedSystemData;

    // Setup the times and frame number
    t_start         = task->t_start;
    t_end           = task->t_end;
    size_t n_frames = data->ComputeNFrames(t_start, t_end);
    if (task->storeInitialState) n_frames++;

    // Setup the single simulation
    TWMCResults *res  = new TWMCResults();
    unsigned int seed = task->rngSeed;
    res->SetId(task->id);
    auto initialCondition = task->initialCondition;

    // Setup the random number generation
    std::mt19937 gen(seed);
    std::normal_distribution<> normal(0, 1);  // mean = 0, std = 1;

    // Generate noisy Matrices
    bool updateMats = false;
    // End handling of noise matrices

    MatrixCXd beta_t = MatrixCXd(nx, 3 * ny);
    switch (initialCondition) {
      case TWMCTaskData::InitialConditions::ReadFromSettings:
        beta_t = data->beta_init_val->Generate(gen);
        randCMat(&tmpRand, gen, normal);
        if (data->beta_init_sigma_val != 0) {
          beta_t = beta_t.array() +
                   data->beta_init_sigma_val->Generate(gen).array() *
                       tmpRand.array();
        } else {
          beta_t += temp_gamma * tmpRand;
        }
        t = data->t_start;
        // Generate noisy Matrices both cases
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          U          = data->U->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::U_Noise, U, 1, {nx, ny, 3});
        }
        if (data->E->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          E          = data->E->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, E, 1, {nx, ny, 3});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          F          = data->F->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::F_Noise, F, 1, {nx, ny, 3});
        }
        break;

      case TWMCTaskData::InitialConditions::FixedPoint:
        beta_t = data->beta_init_val->GenerateNoNoise();

        // Generate noisy Matrices both cases
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          U          = data->U->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::U_Noise, U, 1, {nx, ny, 3});
        }
        if (data->E->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          E          = data->omega->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, E, 1, {nx, ny, 3});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          F          = data->F->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::F_Noise, F, 1, {nx, ny, 3});
        }
        break;

      case TWMCTaskData::InitialConditions::ReadFromPreviousData:
        const auto tmp =
            task->prevData->GetDataset<std::vector<complex_p>>(TWMCData::traj);
        beta_t =
            MatrixCXd(data->beta_init_val->rows(), data->beta_init_val->cols());
        complex_p *beta_t_data = beta_t.data();
        std::memcpy(beta_t_data, &tmp[tmp.size() - nx * ny],
                    sizeof(complex_p) * nx * ny);

        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          U          = task->prevData->GetDataset<MatrixCXd>(TWMCData::U_Noise);
          res->AddDataset<MatrixCXd>(TWMCData::U_Noise, U, 1, {nx, ny, 3});
        }
        if (data->E->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          E = task->prevData->GetDataset<MatrixCXd>(TWMCData::Delta_Noise);
          res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, E, 1, {nx, ny, 3});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          F          = task->prevData->GetDataset<MatrixCXd>(TWMCData::F_Noise);
          res->AddDataset<MatrixCXd>(TWMCData::F_Noise, F, 1, {nx, ny, 3});
        }
    }
    // Merge the three variants in a single for Delta

    if (updateMats) {
      delta = - E.array() + data->detuning;
      real_step_linear = (-ij * delta.array() - data->gamma_val / 2.0);
    }

    // Initialize the beta value to the starting value.
    bool hasTimeDep = data->F->HasTimeDependence();
    size_t tCache;

    auto dt4 = sqrt(data->gamma_val * data->dt / 4.0);
    auto dt  = data->dt;
    std::vector<complex_p> res_betat(nx * ny * cellSz * n_frames);

    int i_step  = 0;
    size_t i_frame = 0;
    double t_frame = 0;

    // Save the initial state if needed
    if (task->storeInitialState) {
      if ((i_step % data->frame_steps == 0) && i_frame < n_frames) {
        size_t size     = nx * ny * cellSz;
        complex_p *data = beta_t.data();
        std::memcpy(&res_betat[i_frame * size], data, sizeof(complex_p) * size);

        i_frame = i_frame + 1;
      }
    }

    while (t <= data->t_end) {
      // Compute F
      if (hasTimeDep) {
        auto Fdata = data->F->GetAtTimeWithSuggestion(t, tCache);
        tCache     = get<0>(Fdata);
        F          = get<1>(Fdata);
      }

      // Compute the a_t, that is used for the kai in the heun scheme
      a_t = ((real_step_linear.array() +
              ij * U.array() * (beta_t.array().abs2() - 1.0)) *
                 beta_t.array() -
             ij * (beta_t * coupling_mat).array() + ij * F.array()) *
            dt;
      randCMat(&tmpRand, gen, normal);
      kai_t = beta_t.array() + a_t.array() + dt4 * tmpRand.array();

      beta_t = kai_t;
      t += data->dt;
      i_step++;

      // Print the data
      if ((i_step % data->frame_steps == 0) && i_frame < n_frames) {
        size_t size     = nx * ny * 3;
        complex_p *data = beta_t.data();
        std::memcpy(&res_betat[i_frame * size], data, sizeof(complex_p) * size);

        i_frame = i_frame + 1; t_frame = t;
        if (i_frame == n_frames) break;
      }
    }

    if (i_frame < n_frames) {
      std::cerr << " error: was supposed to have " << n_frames << " but got "
                << i_frame << " frames " << endl;
      std::cerr << " last frame was at time: " << t_frame << " and simulation ended at " << t << endl;
      res_betat.resize(i_frame*nx*ny*cellSz);
      n_frames = i_frame;
    }


    res->AddDataset<std::vector<complex_p>>(
        TWMCData::traj, std::move(res_betat), n_frames, {nx, ny, 3});
    res->extraDataMemory[0] = task->t_start;
    res->extraDataMemory[1] = t;

    allResults.push_back(std::unique_ptr<TaskResults>(res));
  }
  return allResults;
};

float TWMCLiebSolver::ApproximateComputationProgress() const {
  return float((t - t_start) / t_end);
}

// ************************************* //
// ****** Local Utilituy Methods  ****** //
// ************************************* //

complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm) {
  return (norm(gen) + ij * norm(gen));
}

void randCMat(MatrixCXd *mat, std::mt19937 &gen,
              std::normal_distribution<> norm) {
  complex_p *vals = mat->data();
  size_t dim      = mat->rows() * mat->cols();

  for (size_t i = 0; i < dim; i++) { vals[i] = norm(gen) + ij * norm(gen); }
}
