//
//  TWMCThermoSimulation.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCThermoSimulation.hpp"

#include "NoisyMatrix.hpp"
#include "Settings.hpp"
#include "TWMCSimData.hpp"
#include "TWMCThermoResults.hpp"

#include <fftw3.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>


using namespace std;


// Local Utility Methods
inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm);


TWMCThermoSimulation::TWMCThermoSimulation(const TWMCSimData* TaskData)
{
    data = TaskData;
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
    
    if (TaskData->dimension == TWMCSimData::Dimension::D1 || TaskData->dimension == TWMCSimData::Dimension::D0 )
    {
        plan->forward_fft = fftw_plan_dft_1d(int(nx*ny),
                                             reinterpret_cast<fftw_complex*>(f_in_ptr),
                                             reinterpret_cast<fftw_complex*>(f_out_ptr),
                                             FFTW_FORWARD,
                                             FFTW_MEASURE);
        plan->inverse_fft = fftw_plan_dft_1d(int(nx*ny),
                                             reinterpret_cast<fftw_complex*>(i_in_ptr),
                                             reinterpret_cast<fftw_complex*>(i_out_ptr),
                                             FFTW_BACKWARD,
                                             FFTW_MEASURE);
    }
    else if (TaskData->dimension == TWMCSimData::Dimension::D2)
    {
        plan->forward_fft = fftw_plan_dft_2d(int(nx), int(ny),
                                             reinterpret_cast<fftw_complex*>(f_in_ptr),
                                             reinterpret_cast<fftw_complex*>(f_out_ptr),
                                             FFTW_FORWARD,
                                             FFTW_MEASURE);
        plan->inverse_fft = fftw_plan_dft_2d(int(nx), int(ny),
                                             reinterpret_cast<fftw_complex*>(i_in_ptr),
                                             reinterpret_cast<fftw_complex*>(i_out_ptr),
                                             FFTW_BACKWARD,
                                             FFTW_MEASURE);
    }
    
    // Thermo Temporary Vars
    x = MatrixRXd(nx, ny);
    p = MatrixRXd(nx, ny);

    // dU_t = MatrixCXd(nx, ny);
    // deltaU = MatrixCXd(nx, ny);
    // dQ_t = MatrixCXd(nx, ny);
    // deltaQ = MatrixCXd(nx, ny);

    dW_t = MatrixCXd(nx, ny);
    dA_t = MatrixCXd(nx, ny);

    deltaW = MatrixCXd(nx, ny);
    deltaA = MatrixCXd(nx, ny);

//    dH_dt = MatrixCXd(nx, ny);
//    dH_dx = MatrixCXd(nx, ny);
//    dH_dp = MatrixCXd(nx, ny);
//    ddH_dxx = MatrixCXd(nx, ny);
//    ddH_dpp = MatrixCXd(nx, ny);

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
    res = new TWMCThermoResults(data);
}

TWMCThermoSimulation::~TWMCThermoSimulation()
{
    delete plan;
    delete res;
}

void TWMCThermoSimulation::Setup(TaskData* TaskData)
{
    
}

void TWMCThermoSimulation::Initialize(unsigned int _seed, size_t resultId)
{
    seed = _seed;
    res->SetId(resultId);
    initialCondition = InitialConditions::ReadFromSettings;
}

void TWMCThermoSimulation::Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0, size_t resultId)
{
    seed = _seed;
    res->SetId(resultId);
    initialCondition = InitialConditions::FixedPoint;
    beta_t_init = beta_init;
    t = t0;
}


//
// The Truncated Wigner Evolution Method.
//
void TWMCThermoSimulation::Compute()
{
    // Setup the random number generation
    std::mt19937 gen(seed);
    std::normal_distribution<> normal(0,1); // mean = 0, std = 1;
    
    // Generate noisy Matrices
    bool updateMats = false;
    if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        updateMats=true;
        U = data->U->Generate(gen);
    }
    if (data->omega->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        updateMats=true;
        omega = data->omega->Generate(gen);
    }
    if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        updateMats=true;
        F = data->F->Generate(gen);
    }
    if (updateMats)
    {
        real_step_linear = (-ij*omega - data->gamma/2.0);
    }
    // End handling of noise matrices

    MatrixCXd beta_t;
    switch (initialCondition)
    {
        case ReadFromSettings:
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
        case FixedPoint:
            beta_t = beta_t_init;
            break;
        default:
            break;
    }
    
    int i_step = 0;
    int i_frame = 0;
    int frame_steps = floor(data->dt_obs/data->dt);
    
    // Initialize the beta value to the starting value.
    // deltaU = MatrixCXd::Zero(nx,ny);
    deltaW = MatrixCXd::Zero(nx,ny);
    // deltaQ = MatrixCXd::Zero(nx,ny);
    deltaA = MatrixCXd::Zero(nx,ny);

    plan->fft_i_out = beta_t;

    size_t tCache = 0;

    MatrixCXd F_t;
    MatrixCXd F_told = data->F->GetAtTime(t);
    MatrixCXd F_der;

    while (t<data->t_end)
    {
        // Compute F
        auto Fdata = data->F->GetAtTimeWithSuggestion(t, tCache);
        tCache = get<0>(Fdata);

        F_t = get<1>(Fdata);

        F_der = (F_t-F_told)/data->dt;

        // First compute the Fourier Transform of the previous state
        {
            plan->fft_f_in = beta_t;
            fftw_execute(plan->forward_fft);
            
            // Now in plan.fft_f_out I have the beta_t transformed.
            // I apply the J step
            plan->fft_i_in = k_step_linear.array() * plan->fft_f_out.array();
            fftw_execute(plan->inverse_fft);
            plan->fft_i_out = plan->fft_i_out/fft_norm_factor;
        }
        // Compute the a_t, that is used for the kai in the heun scheme
        a_t = ((real_step_linear.array() + ij*U.array()*(beta_t.array().abs2()-1.0) )*beta_t.array() + plan->fft_i_out.array() + (ij*F_t.array()))*data->dt;
        randCMat(&tmpRand, gen, normal);
        noise = sqrt(data->gamma_val*data->dt/4.0)*tmpRand.array();
        kai_t = beta_t.array() + a_t.array() + noise.array();
        
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

        F_told = F_t;

//        deltaU += dU_t;
//        deltaQ += dQ_t;

        // Print the data
        if((i_step % frame_steps ==0 ) && i_frame < data->n_frames)
        {
            size_t size = res->nx*res->ny;
            complex_p* dataBeta = beta_t.data();
            complex_p* dataWork = deltaW.data();
            complex_p* dataArea = deltaA.data();

            for (unsigned j= 0; j < size; j++)
            {
                res->beta_t[i_frame*size + j] = dataBeta[j];
                res->work_t[i_frame*size + j] = real(dataWork[j]);
                res->area_t[i_frame*size + j] = real(dataArea[j]);
            }
            i_frame = i_frame + 1;

            deltaW = MatrixCXd::Zero(nx,ny);
            deltaA = MatrixCXd::Zero(nx,ny);
            //deltaQ = MatrixCXd::Zero(nx,ny);
            //deltaU = MatrixCXd::Zero(nx,ny);
        }

        t += data->dt;
        i_step++;
    }
};

TaskResults* TWMCThermoSimulation::GetResults() const
{
    return res;
}

float TWMCThermoSimulation::ApproximateComputationProgress() const
{
    return (t-data->t_end)/(data->t_end-data->t_start)*100.0;
}

// ************************************* //
// ****** Local Utilituy Methods  ****** //
// ************************************* //

complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm)
{
    return (norm(gen)+ij*norm(gen));
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
