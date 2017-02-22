//
//  TWMC_evolve.cpp
//  TWMC-test
//
//  Created by Filippo Vicentini on 19/11/16.
//  Copyright © 2016 Filippo Vicentini. All rights reserved.
//

#include "TWMC_evolve.hpp"

#include <iostream>
#include <random>
#include <algorithm>
#include <functional>

#include <fftw3.h>


using namespace std;

const complex_p ij(0.0,1.0);


inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm)
{
    return (norm(gen)+ij*norm(gen));
}

inline MatrixCXd randCMat(size_t nx, size_t ny, std::mt19937 &gen, std::normal_distribution<> norm)
{
    MatrixCXd mat = MatrixCXd::Zero(nx, ny);
    complex_p* vals = mat.data();
    size_t dim=nx*ny;
    
    for (size_t i =0; i<dim ; i++)
    {
        vals[i] = norm(gen)+ij*norm(gen);
    }
    return mat;
}


void printVector(complex_p* vec, int size, string name="")
{
    cout << "----vec: " << name << " -----"<<endl;
#pragma clang loop vectorize(disable)
    for (int i = 0; i != size; i++)
    {
        cout << i << " | " << vec[i] << endl;
    }
    cout <<endl;
}
/* END Utility Methods */


//
// The Truncated Wigner Evolution Method.
//
void TWMC_Evolve_Parallel(size_t th_id, TWMC_Data &dat, TWMC_Results &res, TWMC_FFTW_plans &plan)
{
    // Prepare the output
    res.n = dat.n_frames;
    // Setup the random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> normal(0,1);
    
    // Precompute the  Fourier Space evolution term.
    MatrixCXd k_step_linear = MatrixCXd::Zero(dat.nx, dat.ny);
    MatrixCXd real_step_linear = (-ij*dat.omega - dat.gamma/2.0);

    // If we have a 1D system, then we put to 0 it's contribution of the cosinus (1D/2D) code.
    double flag1DNx = (dat.nx==1) ? 0.0 : 1.0;
    double flag1DNy = (dat.ny==1) ? 0.0 : 1.0;
    for (size_t i = 0, nRows = k_step_linear.rows(), nCols = k_step_linear.cols(); i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            k_step_linear(i,j) = -ij*2.0*dat.J_val*(flag1DNx*cos(2.0*M_PI/double(dat.nx)*double(i)) + flag1DNy*cos(2.0*M_PI/double(dat.ny)*double(j)));
        }
    }
    
    complex_p temp_gamma = sqrt(dat.gamma_val*dat.dt/4.0);
    complex_p idt = ij*dat.dt;

    // Initial iteration values
    float_p t = 0;
    int i_step = 0;
    int i_frame = 0;
    int frame_steps = floor(dat.dt_obs/dat.dt);
    
    // Initialize the beta value to the starting value.
    MatrixCXd beta_t = dat.beta_init + temp_gamma*randCMat(dat.nx, dat.ny, gen,normal);
  
    // If we are 1D then do not normalize, if we are 2D then normalize by nx*ny after each FFT cycle.
    float_p fft_norm_factor = dat.nxy; //(dat.nx == dat.nxy || dat.ny == dat.nxy) ? 1.0 : dat.nxy;
    bool is2D = (dat.nx == dat.nxy || dat.ny == dat.nxy) ? false : true;
    
    plan.fft_i_out = beta_t;

    MatrixCXd kai_t = MatrixCXd::Zero(dat.nx, dat.ny);
    MatrixCXd a_t = MatrixCXd::Zero(dat.nx, dat.ny);
    MatrixCXd a_kai_t = MatrixCXd::Zero(dat.nx, dat.ny);
    
    while (t<dat.t_end)
    {
        // First compute the Fourier Transform of the previous state
        {
            plan.fft_f_in = beta_t;
            fftw_execute(plan.forward_fft);
            
            // Now in plan.fft_f_out I have the beta_t transformed.
            // I apply the J step
            plan.fft_i_in = k_step_linear.array() * plan.fft_f_out.array();
            fftw_execute(plan.inverse_fft);
            plan.fft_i_out = plan.fft_i_out/fft_norm_factor;
        }
        // Compute the a_t, that is used for the kai in the heun scheme
        a_t = ((real_step_linear.array() + ij*dat.U.array()*(beta_t.array().abs2()-1.0) )*beta_t.array() + plan.fft_i_out.array() + (ij*dat.F_val))*dat.dt;
        kai_t = beta_t.array() + a_t.array() + sqrt(dat.gamma_val*dat.dt/4.0)*randCMat(dat.nx, dat.ny, gen, normal).array();
        
        // Now compute the a_kai_t, repeating the previous procedure
        {
            // First compute the fourier transform of the 'previously new state'
            plan.fft_f_in = kai_t;
            fftw_execute(plan.forward_fft);
            
            // Now in plan.fft_f_out I have the beta_t transformed.
            // I apply the J step
            plan.fft_i_in = k_step_linear.array() * plan.fft_f_out.array();
            fftw_execute(plan.inverse_fft);
            plan.fft_i_out = plan.fft_i_out/fft_norm_factor;
        }
        // Compute the a_kai_t
        a_kai_t = ((real_step_linear.array() + ij*dat.U.array()*(kai_t.array().abs2()-1.0) )*kai_t.array() + plan.fft_i_out.array() + (ij*dat.F_val))*dat.dt;
        beta_t = beta_t.array() + 0.5*(a_t.array() + a_kai_t.array() )+ sqrt(dat.gamma_val*dat.dt/4.0)*randCMat(dat.nx, dat.ny, gen, normal).array();
        
        // Print the data
        if((i_step % frame_steps ==0 ) && i_frame < dat.n_frames)
        {
            size_t size = res.nx*res.ny;
            complex_p* data = beta_t.data();

            for (unsigned j= 0; j < size; j++)
            {
                res.beta_t[i_frame*size + j] = data[j];
                //cout << data[j] << "\t";
            }
            //cout << endl<<endl;
            i_frame = i_frame + 1;
            
        }
        t += dat.dt;
        i_step++;
    }
    
    // Not returning any quantity because results are stored in referenced structures.
    return;
};

