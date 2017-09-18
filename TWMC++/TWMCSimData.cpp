//
//  TWMCSimData.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "EigenUtils.hpp"
#include "Settings.hpp"
#include "TWMCSimData.hpp"


TWMCSimData::TWMCSimData(const Settings* settings)
{
    nx = settings->get<size_t>("nx");
    ny = settings->get<size_t>("ny");
    nxy = nx*ny;
    if (nx == 1 && ny == 1)
    {
        dimension = Dimension::D0;
    }
    else if(nx == 1 || ny == 1)
    {
        dimension = Dimension::D1;
        // Always keep as vectors in case of 1D. It is faster.
        if (ny == 1)
        {
            ny = nx;
            nx = 1;
        }
    }
    else
    {
        dimension = Dimension::D2;
    }
    
    // Read the predata
    U_val = settings->get<float_p>("U");
    J_val = settings->get<float_p>("J");
    F_val = settings->get<float_p>("F");
    omega_val = settings->get<float_p>("omega");
    gamma_val = settings->get<float_p>("gamma");
    // TODO: Use complex_p template specialization and add the case of a starting file distrubtion!
    beta_init_val = complex_p(1,0)*double(settings->get<float_p>("beta_init_real"))+ ij*double(settings->get<float_p>("beta_init_imag"));
    beta_init_sigma_val = settings->get<float_p>("beta_init_sigma");

    // The 2D lattice is vectorized in a nx*ny line
    U = InitMatrix(nx, ny, U_val);
    F = InitMatrix(nx, ny, F_val);
    omega = InitMatrix(nx, ny, omega_val);
    gamma = InitMatrix(nx, ny, gamma_val);
    beta_init = InitMatrix(nx, ny, beta_init_val);
    
    //TODO: Read the 2D noise, if they exist;
//    float_p* deltaUTmp = prefs->getValueOrMatrixReal("disorderU", nx, ny);
//    float_p* deltaOmegaTmp = prefs->getValueOrMatrixReal("disorderOmega", nx, ny);
//    float_p* deltaGammaTmp = prefs->getValueOrMatrixReal("disorderGamma", nx, ny);
//    MatrixCXd deltaU = InitMatrix(nx, ny, deltaUTmp); delete[] deltaUTmp;
//    MatrixCXd deltaOmega= InitMatrix(nx, ny, deltaOmegaTmp); delete[] deltaOmegaTmp;
//    MatrixCXd deltaGamma = InitMatrix(nx, ny, deltaGammaTmp); delete[] deltaGammaTmp;
//    U = U + deltaU;
//    omega = omega + deltaOmega;
//    gamma = gamma + deltaGamma;

    n_frames = settings->get<size_t>("n_frames");
    t_start = settings->get<float_p>("t_start");
    t_end = settings->get<float_p>("t_end");
    
    // Compute the timestep
    int timestep_factor = settings->get<int>("timestep_factor");
    float_p vals_array[] = {abs(U_val), abs(J_val), abs(F_val), abs(omega_val)};
    float_p w_max = *max_element(vals_array,vals_array+4);
    int n_times = round(log2((t_end - t_start)*w_max));
    dt = (t_end - t_start)/pow(2,n_times+timestep_factor);
    dt_obs = n_frames == 0 ? 0.0 :(t_end - t_start)/n_frames;
    n_dt = (t_end - t_start)/dt;
    
    // Fix the dt_timestep
    if (dt_obs < dt)
    {
        dt_obs = dt;
        n_frames = n_dt;
    }

}
