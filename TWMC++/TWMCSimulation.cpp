//
//  TWMCSimulation.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCSimulation.hpp"

#include <iostream>
#include <random>
#include <algorithm>
#include <functional>

#include <fftw3.h>
using namespace std;

// Local Utilituy Methods
inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm);


//
// The Truncated Wigner Evolution Method.
//
void TWMCSimulation::Compute()
{
    // Prepare the output
    res->n = data->n_frames;
    // Setup the random number generation
    std::mt19937 gen(seed);
    std::normal_distribution<> normal(0,1); // mean = 0, std = 1;
    MatrixCXd tmpRand = MatrixCXd::Zero(data->nx, data->ny);
    
    // Precompute the  Fourier Space evolution term.
    MatrixCXd k_step_linear = MatrixCXd::Zero(data->nx, data->ny);
    MatrixCXd real_step_linear = (-ij*data->omega - data->gamma/2.0);
    
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
    
    complex_p temp_gamma = sqrt(data->gamma_val*data->dt/4.0);
    
    // Initial iteration values
    float_p t = 0;
    int i_step = 0;
    int i_frame = 0;
    int frame_steps = floor(data->dt_obs/data->dt);
    
    // Initialize the beta value to the starting value.
    MatrixCXd beta_t = data->beta_init;
    
    randCMat(&tmpRand, gen, normal);
    if (data->beta_init_sigma_val != 0)
    {
        beta_t += data->beta_init_sigma_val*tmpRand;
    }
    else
    {
        beta_t = beta_t + temp_gamma*tmpRand;
    }
    
    
    // If we are 1D then do not normalize, if we are 2D then normalize by nx*ny after each FFT cycle.
    float_p fft_norm_factor = data->nxy; //(data->nx == data->nxy || data->ny == data->nxy) ? 1.0 : data->nxy;
    //bool is2D = (data->nx == data->nxy || data->ny == data->nxy) ? false : true;
    
    plan->fft_i_out = beta_t;
    
    MatrixCXd kai_t = MatrixCXd::Zero(data->nx, data->ny);
    MatrixCXd a_t = MatrixCXd::Zero(data->nx, data->ny);
    MatrixCXd a_kai_t = MatrixCXd::Zero(data->nx, data->ny);
    
    while (t<data->t_end)
    {
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
        a_t = ((real_step_linear.array() + ij*data->U.array()*(beta_t.array().abs2()-1.0) )*beta_t.array() + plan->fft_i_out.array() + (ij*data->F_val))*data->dt;
        randCMat(&tmpRand, gen, normal);
        kai_t = beta_t.array() + a_t.array() + sqrt(data->gamma_val*data->dt/4.0)*tmpRand.array();
        
        beta_t = kai_t;
        
        // Print the data
        if((i_step % frame_steps ==0 ) && i_frame < data->n_frames)
        {
            size_t size = res->nx*res->ny;
            complex_p* data = beta_t.data();
            
            for (unsigned j= 0; j < size; j++)
            {
                res->beta_t[i_frame*size + j] = data[j];
            }
            i_frame = i_frame + 1;
        }
        t += data->dt;
        i_step++;
    }
};





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
