//
// Created by Filippo Vicentini on 22/12/17.
//

#include "TWMCBaseSolver.hpp"

#include "TWMC/TWMCResults.hpp"
#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"

#include "Base/NoisyMatrix.hpp"

#include <array>
#include <mutex>

TWMCBaseSolver::TWMCBaseSolver() { nTasksToRequest = 1; }

TWMCBaseSolver::~TWMCBaseSolver() { delete plan; }

inline void randCMat(MatrixCXd *mat, std::mt19937 &gen,
                     std::normal_distribution<> norm);
std::mutex fftw_mutex;

void TWMCBaseSolver::Setup() {
  const TWMCSystemData *data = lastSharedSystemData;
  nx                         = data->nx;
  ny                         = data->ny;

  plan            = new TWMC_FFTW_plans;
  plan->fft_f_in  = MatrixCXd::Zero(nx, ny);
  plan->fft_f_out = MatrixCXd::Zero(nx, ny);
  plan->fft_i_in  = MatrixCXd::Zero(nx, ny);
  plan->fft_i_out = MatrixCXd::Zero(nx, ny);

  complex_p *f_in_ptr  = plan->fft_f_in.data();
  complex_p *f_out_ptr = plan->fft_f_out.data();
  complex_p *i_in_ptr  = plan->fft_i_in.data();
  complex_p *i_out_ptr = plan->fft_i_out.data();

  {
    fftw_mutex.lock();
    if (data->dimension == TWMCSystemData::Dimension::D1 ||
        data->dimension == TWMCSystemData::Dimension::D0) {
      plan->forward_fft = fftw_plan_dft_1d(
          int(nx * ny), reinterpret_cast<fftw_complex *>(f_in_ptr),
          reinterpret_cast<fftw_complex *>(f_out_ptr), FFTW_FORWARD,
          FFTW_MEASURE);
      plan->inverse_fft = fftw_plan_dft_1d(
          int(nx * ny), reinterpret_cast<fftw_complex *>(i_in_ptr),
          reinterpret_cast<fftw_complex *>(i_out_ptr), FFTW_BACKWARD,
          FFTW_MEASURE);
    } else if (data->dimension == TWMCSystemData::Dimension::D2) {
      plan->forward_fft = fftw_plan_dft_2d(
          int(nx), int(ny), reinterpret_cast<fftw_complex *>(f_in_ptr),
          reinterpret_cast<fftw_complex *>(f_out_ptr), FFTW_FORWARD,
          FFTW_MEASURE);
      plan->inverse_fft = fftw_plan_dft_2d(
          int(nx), int(ny), reinterpret_cast<fftw_complex *>(i_in_ptr),
          reinterpret_cast<fftw_complex *>(i_out_ptr), FFTW_BACKWARD,
          FFTW_MEASURE);
    }
    fftw_mutex.unlock();
  }
  // Temporary variables
  temp_gamma    = sqrt(data->gamma_val * data->dt / 4.0);
  tmpRand       = MatrixCXd::Zero(data->nx, data->ny);
  kai_t         = MatrixCXd::Zero(data->nx, data->ny);
  a_t           = MatrixCXd::Zero(data->nx, data->ny);
  k_step_linear = MatrixCXd::Zero(data->nx, data->ny);

  omega            = data->omega->GenerateNoNoise();
  U                = data->U->GenerateNoNoise();
  F                = data->F->GenerateNoNoise();
  real_step_linear = (-ij * omega - data->gamma / 2.0);

  // If we have a 1D system, then we put to 0 it's contribution of the cosinus
  // (1D/2D) code.
  double flag1DNx = (data->nx == 1) ? 0.0 : 1.0;
  double flag1DNy = (data->ny == 1) ? 0.0 : 1.0;
  for (size_t i = 0, nRows = k_step_linear.rows(), nCols = k_step_linear.cols();
       i < nRows; ++i) {
    for (size_t j = 0; j < nCols; ++j) {
      k_step_linear(i, j) =
          -ij * 2.0 * data->J_val *
          (flag1DNx * cos(2.0 * M_PI / double(data->nx) * double(i)) +
           flag1DNy * cos(2.0 * M_PI / double(data->ny) * double(j)));
    }
  }
  // If we are 1D then do not normalize, if we are 2D then normalize by nx*ny
  // after each FFT cycle.
  fft_norm_factor = data->nxy;
}

std::vector<std::unique_ptr<TaskResults>> TWMCBaseSolver::Compute(
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

    std::mt19937 gen(seed);
    std::normal_distribution<> normal(0, 1);  // mean = 0, std = 1;

    bool updateMats = false;

    MatrixCXd beta_t;
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
        // Generate noisy Matrices both cases
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          U          = data->U->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::U_Noise, U, 1, {nx, ny});
        }
        if (data->omega->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          omega      = data->omega->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, omega, 1, {nx, ny});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          F          = data->F->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::F_Noise, F, 1, {nx, ny});
        }
        break;

      case TWMCTaskData::InitialConditions::FixedPoint:
        beta_t = data->beta_init_val->GenerateNoNoise();

        // Generate noisy Matrices both cases
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          U          = data->U->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::U_Noise, U, 1, {nx, ny});
        }
        if (data->omega->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          omega      = data->omega->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, omega, 1, {nx, ny});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          F          = data->F->Generate(gen);
          res->AddDataset<MatrixCXd>(TWMCData::F_Noise, F, 1, {nx, ny});
        }
        // End handling of noise matrices

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
          res->AddDataset<MatrixCXd>(TWMCData::U_Noise, U, 1, {nx, ny});
        }
        if (data->omega->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          omega = task->prevData->GetDataset<MatrixCXd>(TWMCData::Delta_Noise);
          res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, omega, 1, {nx, ny});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None) {
          updateMats = true;
          F          = task->prevData->GetDataset<MatrixCXd>(TWMCData::F_Noise);
          res->AddDataset<MatrixCXd>(TWMCData::F_Noise, F, 1, {nx, ny});
        }

        break;
        /*default:
          break;*/
    }

    if (updateMats) { real_step_linear = (-ij * omega - data->gamma / 2.0); }

    plan->fft_i_out = beta_t;

    auto dt4 = sqrt(data->gamma_val * data->dt / 4.0);
    auto dt  = data->dt;

    TWMC_FFTW_plans &_plan = *plan;

    std::vector<complex_p> res_betat(nx * ny * n_frames);
    int i_step  = 0;
    size_t i_frame = 0;
    double t_frame = 0;

    // Save the initial state if needed
    if (task->storeInitialState) {
      if ((i_step % data->frame_steps == 0) && i_frame < n_frames) {
        size_t size     = nx * ny;
        complex_p *data = beta_t.data();
        std::memcpy(&res_betat[i_frame * size], data, sizeof(complex_p) * size);

        i_frame = i_frame + 1;
      }
    }

    t = t_start;
    while (t <= t_end) {
      // First compute the Fourier Transform of the previous state
      {
        _plan.fft_f_in = beta_t;
        fftw_execute(_plan.forward_fft);

        // Now in plan.fft_f_out I have the beta_t transformed.
        // I apply the J step
        _plan.fft_i_in = k_step_linear.array() * _plan.fft_f_out.array();
        fftw_execute(_plan.inverse_fft);
        _plan.fft_i_out = _plan.fft_i_out / fft_norm_factor;
      }
      // Compute the a_t, that is used for the kai in the heun scheme
      a_t = ((real_step_linear.array() +
              ij * U.array() * (beta_t.array().abs2() - 1.0)) *
                 beta_t.array() +
             _plan.fft_i_out.array() + ij * F.array()) *
            dt;
      randCMat(&tmpRand, gen, normal);
      kai_t = beta_t.array() + a_t.array() + dt4 * tmpRand.array();

      beta_t = kai_t;
      t += data->dt;
      i_step++;

      // Print the data
      if ((i_step % data->frame_steps == 0) && i_frame < n_frames) {
        size_t size     = nx * ny;
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
      res_betat.resize(i_frame*nx*ny);
      n_frames = i_frame;
    }

    res->AddDataset<std::vector<complex_p>>(
        TWMCData::traj, std::move(res_betat), n_frames, {nx, ny});
    res->extraDataMemory[0] = task->t_start;
    res->extraDataMemory[1] = t;
    allResults.push_back(std::unique_ptr<TaskResults>(res));
  }
  return allResults;
}

void randCMat(MatrixCXd *mat, std::mt19937 &gen,
              std::normal_distribution<> norm) {
  complex_p *vals = mat->data();
  size_t dim      = mat->rows() * mat->cols();

  for (size_t i = 0; i < dim; i++) { vals[i] = norm(gen) + ij * norm(gen); }
}

float TWMCBaseSolver::ApproximateComputationProgress() const {
  return float((t - t_start) / t_end);
}