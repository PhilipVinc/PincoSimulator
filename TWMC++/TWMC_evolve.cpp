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
    MatrixCXd k_step = MatrixCXd::Zero(dat.nx, dat.ny);
    MatrixCXd real_step_log = (-ij*dat.omega - dat.gamma/2.0)*dat.dt;
    
    // If we have a 1D system, then we put to 0 it's contribution of the cosinus (1D/2D) code.
    double flag1DNx = (dat.nx==1) ? 0.0 : 1.0;
    double flag1DNy = (dat.ny==1) ? 0.0 : 1.0;
    for (size_t i = 0, nRows = k_step.rows(), nCols = k_step.cols(); i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            k_step(i,j) = exp((-ij*2.0*dat.J_val*(flag1DNx*cos(2.0*M_PI/double(dat.nx)*double(i)) + flag1DNy*cos(2.0*M_PI/double(dat.ny)*double(j))))*dat.dt);
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
    plan.fft_i_out = dat.beta_init + temp_gamma*randCMat(dat.nx, dat.ny, gen,normal);
  
    // If we are 1D then do not normalize, if we are 2D then normalize by nx*ny after each FFT cycle.
    float_p fft_norm_factor = (dat.nx == dat.nxy || dat.ny == dat.nxy) ? 1.0 : dat.nxy;
    
    /*
    cout << "Initial vector = " << plan.fft_i_out << endl << endl;
    
    cout << "k_step = " << k_step << endl << endl;
    cout << "real_step_log = " << real_step_log << endl << endl;
    
    cout << "U =  " << dat.U << endl<<endl;
    cout << "omega =  " << dat.omega << endl<<endl;
    cout << "gamma =  " << dat.gamma << endl<<endl;
    cout << "nxy =  " << dat.nxy << endl<<endl;

    cout << "............................................." << endl;
    plan.fft_f_in = plan.fft_i_out;
    cout << "i= " << plan.fft_f_in << endl;
    fftw_execute(plan.forward_fft);
    plan.fft_i_in = plan.fft_f_out;
    fftw_execute(plan.inverse_fft);
    cout << "o= " << plan.fft_i_out << endl;
    cout << "o= " << plan.fft_i_out/dat.nxy << endl;
    cout << "............................................." << endl;
     */

    
    while (t<dat.t_end)
    {
        //cout << "t= " << t << "     psi= " << plan.fft_i_out << endl;
        //cout <<"norm= " << plan.fft_i_out.cwiseAbs2() << endl << endl;
        
        complex_p tempDrive = ij*dat.F_val*dat.dt;

        // Apply the real-space step, while writing directly to the forward fft register.
        plan.fft_f_in = (real_step_log.array()+idt*dat.U.array()*(plan.fft_i_out.array().abs2() -1.0)).exp()*plan.fft_i_out.array() + tempDrive + temp_gamma*randCMat(dat.nx, dat.ny, gen, normal).array();
        
        // Do the fft, outputting to fft_f_out
        fftw_execute(plan.forward_fft);
                
        // Apply k-space evolution
        plan.fft_i_in = k_step.array()*plan.fft_f_out.array();
        
        // Inverse fft, storing result in fft_i_out
        fftw_execute(plan.inverse_fft);
        
        plan.fft_i_out = plan.fft_i_out/fft_norm_factor;
        
        // Print the data
        if((i_step % frame_steps ==0 ) && i_frame < dat.n_frames)
        {
            size_t size = res.nx*res.ny;
            complex_p* data = plan.fft_i_out.data();

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

