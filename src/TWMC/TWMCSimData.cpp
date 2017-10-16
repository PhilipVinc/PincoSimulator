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
#include "NoisyMatrix.hpp"


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
    
    U = settings->GetMatrix("U", nx, ny);
    F = settings->GetMatrix("F",  nx, ny);
    omega = settings->GetMatrix("omega",  nx, ny);
    //gamma = settings->GetMatrix("gamma",  nx, ny);

    // Read the predata
    //U_val = settings->get<float_p>("U");
    J_val = settings->get<float_p>("J");
    //F_val = settings->get<float_p>("F");
    //omega_val = settings->get<float_p>("omega");
    gamma_val = settings->get<float_p>("gamma");
    
    // TODO: Use complex_p template specialization and add the case of a starting file distrubtion!
    beta_init_val = settings->get<complex_p>("beta_init");
    beta_init_sigma_val = settings->get<float_p>("beta_init_sigma");

    // The 2D lattice is vectorized in a nx*ny line
    //U = InitMatrix(nx, ny, U_val);
    //F = InitMatrix(nx, ny, F_val);
    //omega = InitMatrix(nx, ny, omega_val);
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
    
    float_p timestep = settings->get<float_p>("timestep");
    
    size_t n_times = 0; float_p t = t_start;
    while (t<= t_end)
    {
        n_times++;
        t +=timestep;
    }
    dt = timestep;
    n_dt = n_times;
    frame_steps = ceil(n_times/n_frames);
    dt_obs = dt*frame_steps;
    if ( t_start + dt*frame_steps*n_frames > t_end)
        frame_steps -= 1;
    
    //dt = (t_end - t_start)/pow(2,n_times+timestep_factor);
    //dt_obs = n_frames == 0 ? 0.0 :(t_end - t_start)/n_frames;
    //n_dt = (t_end - t_start)/dt;
    
    // Fix the dt_timestep
    if (dt_obs < dt)
    {
        dt_obs = dt;
        n_frames = n_dt;
    }
    else
    {
        n_frames++;
        t_end = t_end+dt;
    }

}

vector<float_p> TWMCSimData::GetStoredTimes()
{
    vector<float_p> times(n_frames);

    float_p t = t_start; size_t i_step = 0; size_t i_frame = 0;
    while(t<=t_end)
    {
        if((i_step % frame_steps ==0 ) && i_frame < n_frames)
        {
            times[i_frame] = t;
            i_frame ++;
        }
        t+=dt; i_step ++;
    }
    return times;
}


vector<vector<float_p>> TWMCSimData::GetStoredVariableEvolution(NoisyMatrix* mat)
{
    vector<vector<float_p>> result(n_frames, vector<float_p>(nx*ny));

    float_p t = t_start; size_t i_step = 0; size_t i_frame = 0;
    while(t<=t_end)
    {
        if((i_step % frame_steps ==0 ) && i_frame < n_frames)
        {
            MatrixCXd m = mat->GetAtTime(t);
            complex_p* data = m.data();
            for (int kk=0; kk != nx*ny; kk++) {
                result[i_frame][kk] = data[kk].real();
            }
            i_frame ++;
        }
        t+=dt; i_step ++;
    }
    return result;
}
