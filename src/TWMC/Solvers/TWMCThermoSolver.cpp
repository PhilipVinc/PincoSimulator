//
// Created by Filippo Vicentini on 25/12/17.
//

#include "TWMCThermoSolver.hpp"

#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"
#include "TWMC/TWMCResults.hpp"

#include "Base/NoisyMatrix.hpp"

#include <mutex>

TWMCThermoSolver::TWMCThermoSolver() {
    nTasksToRequest = 1;
}

TWMCThermoSolver::~TWMCThermoSolver() {

}

inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
extern std::mutex fftw_mutex;

void TWMCThermoSolver::Setup() {
    const TWMCSystemData* data = lastSharedSystemData;

    nx = data->nx;
    ny = data->ny;

    plan = new TWMC_FFTW_plans;
    plan->fft_f_in = MatrixCXd::Zero(nx, ny);
    plan->fft_f_out = MatrixCXd::Zero(nx, ny);
    plan->fft_i_in = MatrixCXd::Zero(nx, ny);
    plan->fft_i_out = MatrixCXd::Zero(nx, ny);

    complex_p* f_in_ptr = plan->fft_f_in.data();
    complex_p* f_out_ptr = plan->fft_f_out.data();
    complex_p* i_in_ptr = plan->fft_i_in.data();
    complex_p* i_out_ptr = plan->fft_i_out.data();

    {
        fftw_mutex.lock();
        if (data->dimension == TWMCSystemData::Dimension::D1 ||
            data->dimension == TWMCSystemData::Dimension::D0) {
            plan->forward_fft = fftw_plan_dft_1d(int(nx * ny),
                                                 reinterpret_cast<fftw_complex *>(f_in_ptr),
                                                 reinterpret_cast<fftw_complex *>(f_out_ptr),
                                                 FFTW_FORWARD,
                                                 FFTW_MEASURE);
            plan->inverse_fft = fftw_plan_dft_1d(int(nx * ny),
                                                 reinterpret_cast<fftw_complex *>(i_in_ptr),
                                                 reinterpret_cast<fftw_complex *>(i_out_ptr),
                                                 FFTW_BACKWARD,
                                                 FFTW_MEASURE);
        } else if (data->dimension == TWMCSystemData::Dimension::D2) {
            plan->forward_fft = fftw_plan_dft_2d(int(nx), int(ny),
                                                 reinterpret_cast<fftw_complex *>(f_in_ptr),
                                                 reinterpret_cast<fftw_complex *>(f_out_ptr),
                                                 FFTW_FORWARD,
                                                 FFTW_MEASURE);
            plan->inverse_fft = fftw_plan_dft_2d(int(nx), int(ny),
                                                 reinterpret_cast<fftw_complex *>(i_in_ptr),
                                                 reinterpret_cast<fftw_complex *>(i_out_ptr),
                                                 FFTW_BACKWARD,
                                                 FFTW_MEASURE);
        }
        fftw_mutex.unlock();
    }
    // Temporary variables
    temp_gamma = sqrt(data->gamma_val*data->dt/4.0);
    tmpRand = MatrixCXd::Zero(data->nx, data->ny);
    kai_t = MatrixCXd::Zero(data->nx, data->ny);
    a_t = MatrixCXd::Zero(data->nx, data->ny);
    k_step_linear = MatrixCXd::Zero(data->nx, data->ny);

    omega = data->omega->GenerateNoNoise();
    U = data->U->GenerateNoNoise();
    F = data->F->GenerateNoNoise();
    real_step_linear = (-ij*omega - data->gamma/2.0);

    // If we have a 1D system, then we put to 0 it's contribution of the cosinus (1D/2D) code.
    double flag1DNx = (data->nx==1) ? 0.0 : 1.0;
    double flag1DNy = (data->ny==1) ? 0.0 : 1.0;
    for (size_t i = 0, nRows = k_step_linear.rows(), nCols = k_step_linear.cols(); i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            k_step_linear(i,j) = -ij*2.0*data->J_val*(flag1DNx*cos(2.0*M_PI/double(data->nx)*double(i)) + flag1DNy*cos(2.0*M_PI/double(data->ny)*double(j)));
        }
    }
    // If we are 1D then do not normalize, if we are 2D then normalize by nx*ny after each FFT cycle.
    fft_norm_factor = data->nxy;
}

std::vector<std::unique_ptr<TaskResults>> TWMCThermoSolver::Compute(const std::vector<std::unique_ptr<TaskData>>& tasks)
{
    std::vector<std::unique_ptr<TaskResults>> allResults;
    for (size_t i =0; i < tasks.size(); i++ )
    {
        // TODO Fix this cast
        TWMCTaskData* task = static_cast<TWMCTaskData*>(tasks[i].get());

        // Check if the system is always the same
        if (task->systemData.get() != lastSharedSystemData)
        {
            lastSharedSystemData = task->systemData.get();
            Setup();
        }

        // Setup the single simulation
        TWMCResults* res = new TWMCResults(task);
        //allResults.push_back(res);
        unsigned int seed = task->rngSeed;
        res->SetId(seed);
        auto initialCondition = TWMCTaskData::InitialConditions::ReadFromSettings;

        std::mt19937 gen(seed);
        std::normal_distribution<> normal(0,1); // mean = 0, std = 1;

        const TWMCSystemData* data = lastSharedSystemData;

        // Generate noisy Matrices
        // Generate noisy Matrices
        bool updateMats = false;
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            U = data->U->Generate(gen);
            res->AddComplexMatrixDataset(variables::U_Noise,
                                         std::vector<complex_p>(U.data(), U.data() + U.size()),
                                         1, {nx,ny});
        }
        if (data->omega->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            omega = data->omega->Generate(gen);
            res->AddComplexMatrixDataset(variables::Delta_Noise,
                                         std::vector<complex_p>(omega.data(), omega.data() + omega.size()),
                                         1, {nx,ny});
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            F = data->F->Generate(gen);
            res->AddComplexMatrixDataset(variables::F_Noise,
                                         std::vector<complex_p>(F.data(), F.data() + F.size()),
                                         1, {nx,ny});
        }
        if (updateMats)
        {
            real_step_linear = (-ij*omega - data->gamma/2.0);
        }
        // End handling of noise matrices

        MatrixCXd beta_t;
        switch (initialCondition)
        {
            case TWMCTaskData::InitialConditions::ReadFromSettings:
                beta_t = data->beta_init;
                randCMat(&tmpRand, gen, normal);
                if (data->beta_init_sigma_val != 0)
                {
                    beta_t += data->beta_init_sigma_val*tmpRand;
                }
                else
                {
                    beta_t += temp_gamma*tmpRand;
                }
                t = data->t_start;
                break;
            case TWMCTaskData::InitialConditions::FixedPoint:
                beta_t = beta_t_init;
                break;
            default:
                break;
        }

        t_start = data->t_start;
        t_end = data->t_end;

        int i_step = 0;
        int i_frame = 0;
        int frame_steps = floor(data->dt_obs/data->dt);

        // Initialize the beta value to the starting value.
        // deltaU = MatrixCXd::Zero(nx,ny);
        deltaW = MatrixCXd::Zero(nx,ny);
        // deltaQ = MatrixCXd::Zero(nx,ny);
        deltaA = MatrixCXd::Zero(nx,ny);

        plan->fft_i_out = beta_t;

        auto dt4 = sqrt(data->gamma_val*data->dt/4.0);
        auto dt = data->dt;

        TWMC_FFTW_plans& _plan = *plan;

        size_t tCache = 0;

        MatrixCXd F_t;
        MatrixCXd F_told = data->F->GetAtTime(t);
        MatrixCXd F_der;
        std::vector<complex_p> res_betat(nx*ny*data->n_frames);
        std::vector<float_p> res_workt(nx*ny*data->n_frames);
        std::vector<float_p> res_areat(nx*ny*data->n_frames);

        while (t<=t_end)
        {

            // Compute F
            auto Fdata = data->F->GetAtTimeWithSuggestion(t, tCache);
            tCache = get<0>(Fdata);
            F_t = get<1>(Fdata);
            F_der = (F_t-F_told)/data->dt;

            // First compute the Fourier Transform of the previous state
            {
                _plan.fft_f_in = beta_t;
                fftw_execute(_plan.forward_fft);

                // Now in plan.fft_f_out I have the beta_t transformed.
                // I apply the J step
                _plan.fft_i_in = k_step_linear.array() * _plan.fft_f_out.array();
                fftw_execute(_plan.inverse_fft);
                _plan.fft_i_out = _plan.fft_i_out/fft_norm_factor;
            }
            // Compute the a_t, that is used for the kai in the heun scheme
            a_t = ((real_step_linear.array() + ij*U.array()*(beta_t.array().abs2()-1.0) )*beta_t.array() + _plan.fft_i_out.array() + ij*F_t.array())*dt;
            randCMat(&tmpRand, gen, normal);
            kai_t = beta_t.array() + a_t.array() + dt4*tmpRand.array();

            beta_t = kai_t;

            // Thermo Stuff
            x = beta_t.real();  p = beta_t.imag();
            dW_t = (2.0l*data->dt)*x.array()*F_der.array();
            dA_t = (data->dt*F_der.array())*(beta_t.cwiseAbs2().array() - 0.5);

            deltaW += dW_t;
            deltaA += dA_t;
            /*
            dWx = noise.real(); dWp = noise.imag();

            dH_dt = x.array()*F_der.array();
            dH_dx = -omega.array()*x.array() + U.array()*(x.array()*x.array()*x.array() +
                                                          x.array()*p.array()*p.array() -
                                                          x.array()); // -F_t.array();
            dH_dp = -omega.array()*p.array() + U.array()*(p.array()*p.array()*p.array() +
                                                          p.array()*x.array()*x.array() -
                                                          p.array());
            ddH_dxx = -omega.array() + U.array()*(3.0l*x.array()*x.array() + p.array()*p.array() - 1.0l);
            ddH_dpp = -omega.array() + U.array()*(3.0l*p.array()*p.array() + x.array()*x.array() - 1.0l);

            dU_t = -omega.array()/2.0l*data->dt*(dH_dx.array()*x.array() +
                                                dH_dp.array()*p.array()) -
                    data->dt*dH_dt.array() + dH_dx.array()*dWx.array() +
                    dH_dp.array()*dWp.array() - (data->dt*data->gamma_val/8.0l*(ddH_dxx + ddH_dpp)).array();
            //dQ_t = ;
            */


            // Print the data
            if((i_step % data->frame_steps ==0 ) && i_frame < data->n_frames)
            {
                size_t size = res->nx*res->ny;
                complex_p* data = beta_t.data();
                memcpy(&res_betat[i_frame*size], data, sizeof(complex_p)*size);

                complex_p* dataWork = deltaW.data();
                complex_p* dataArea = deltaA.data();
                for (unsigned j= 0; j < size; j++)
                {
                    res_workt[i_frame*size + j] = real(dataWork[j]);
                    res_areat[i_frame*size + j] = real(dataArea[j]);
                }
                deltaW = MatrixCXd::Zero(nx,ny);
                deltaA = MatrixCXd::Zero(nx,ny);


                i_frame = i_frame + 1;
            }
            t += data->dt;
            i_step++;
        }
        res->AddComplexMatrixDataset(variables::traj, res_betat, data->n_frames, {nx,ny});
        res->AddRealMatrixDataset(variables::work, res_workt, data->n_frames, {nx,ny});
        res->AddRealMatrixDataset(variables::area, res_areat, data->n_frames, {nx,ny});
        allResults.push_back(std::unique_ptr<TaskResults>(res));
    }
    return allResults;
}


void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm)
{
    complex_p* vals = mat->data();
    size_t dim=mat->rows()*mat->cols();

    for (size_t i =0; i<dim ; i++)
    {
        vals[i] = norm(gen)+ij*norm(gen);
    }
}

float TWMCThermoSolver::ApproximateComputationProgress() const{
    return float((t-t_start)/t_end);
}