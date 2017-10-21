//
//  TWMCSimData.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCSimData.hpp"

#include "EigenUtils.hpp"
#include "NoisyMatrix.hpp"
#include "Settings.hpp"

#include <iostream>


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
	    // if we have a 1xNy lattice convert it to a Ny x 1 lattice, because it's more efficient.
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

    // Read the Uniform Values
    J_val = settings->get<float_p>("J");
    gamma_val = settings->get<float_p>("gamma");
    
    // TODO: Use complex_p template specialization and add the case of a starting file distrubtion!
    beta_init_val = settings->get<complex_p>("beta_init");
    beta_init_sigma_val = settings->get<float_p>("beta_init_sigma");

    // The 2D lattice is vectorized in a nx*ny line
    gamma = InitMatrix(nx, ny, gamma_val);
	beta_init = InitMatrix(nx, ny, beta_init_val);

    n_frames = settings->get<size_t>("n_frames");
    t_start = settings->get<float_p>("t_start");
    t_end = settings->get<float_p>("t_end");

    if (n_frames == 0) {
        std::cerr << "Must have a non-zero, integer number of frames! Exiting." << endl;
        exit(-1);
    }

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

vector<float_p> TWMCSimData::GetStoredTimes() const
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


vector<vector<float_p>> TWMCSimData::GetStoredVariableEvolution(const NoisyMatrix* mat) const
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
