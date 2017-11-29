//
// Created by Filippo Vicentini on 27/11/2017.
//

#include "TWLiebSimData.hpp"

#include "EigenUtils.hpp"
#include "NoisyMatrix.hpp"
#include "Settings.hpp"

#include <iostream>


TWLiebSimData::TWLiebSimData(const Settings* settings)
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

    U_A = settings->GetMatrix("U_A", nx, ny);
    U_B = settings->GetMatrix("U_B", nx, ny);
    U_C = settings->GetMatrix("U_C", nx, ny);
    F_A = settings->GetMatrix("F_A",  nx, ny);
    F_B = settings->GetMatrix("F_B",  nx, ny);
    F_C = settings->GetMatrix("F_C",  nx, ny);
    E_A = settings->GetMatrix("E_A",  nx, ny);
    E_B = settings->GetMatrix("E_B",  nx, ny);
    E_C = settings->GetMatrix("E_C",  nx, ny);
    detuning = settings->get<float_p>("detuning");

    //gamma = settings->GetMatrix("gamma",  nx, ny);

    // Read the Uniform Values
    J_AB_val = settings->get<float_p>("J_AB");
    J_BC_val = settings->get<float_p>("J_BC");
    gamma_val = settings->get<float_p>("gamma");

    // TODO: Use complex_p template specialization and add the case of a starting file distrubtion!
    beta_init_val = settings->get<complex_p>("beta_init");
    beta_init_sigma_val = settings->get<float_p>("beta_init_sigma");

    // The 2D lattice is vectorized in a nx*ny line
    gamma = InitMatrix(nx, 3*ny, gamma_val);
    beta_init = InitMatrix(nx, 3*ny, beta_init_val);

    n_frames = settings->get<size_t>("n_frames");
    t_start = settings->get<float_p>("t_start");
    t_end = settings->get<float_p>("t_end");

    PBC = settings->get<bool>("PBC", false);

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

vector<float_p> TWLiebSimData::GetStoredTimes() const
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


vector<vector<float_p>> TWLiebSimData::GetStoredVariableEvolution(const NoisyMatrix* mat) const
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