//
// Created by Filippo Vicentini on 22/12/17.
//

#include "TWMCSystemData.hpp"
#include "Base/Utils/EigenUtils.hpp"
#include "Base/NoisyMatrix.hpp"
#include "Base/Settings.hpp"

#include <iostream>


TWMCSystemData::TWMCSystemData(const Settings* settings)
{
	nx = settings->get<size_t>("nx");
	ny = settings->get<size_t>("ny");

	cellSz = settings->get<size_t>("cellSz",1);
    latticeName = settings->get<std::string>("lattice");
    if (latticeName == "lieb")
        cellSz = 3;

	nxy = nx*ny;
    PBC = settings->get<bool>("PBC", false);

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

	U = settings->GetMatrix("U", nx, ny, cellSz);
	F = settings->GetMatrix("F",  nx, ny, cellSz);
    omega = settings->GetMatrix("omega",  nx, ny, cellSz);
    E = settings->GetMatrix("E",  nx, ny, cellSz);
	//gamma = settings->GetMatrix("gamma",  nx, ny);

	// Read the Uniform Values
    detuning = settings->get<float_p>("detuning");
    J_AB_val = settings->get<float_p>("J_AB");
    J_BC_val = settings->get<float_p>("J_BC");
	J_val = settings->get<float_p>("J");
	gamma_val = settings->get<float_p>("gamma");

	// TODO: Use complex_p template specialization and add the case of a starting file distrubtion!
	beta_init_val = settings->get<complex_p>("beta_init");
	beta_init_sigma_val = settings->get<float_p>("beta_init_sigma");

	// The 2D lattice is vectorized in a nx*ny line
	gamma = InitMatrix(nx, ny, cellSz, gamma_val);
	beta_init = InitMatrix(nx, ny, cellSz, beta_init_val);

	n_frames = settings->get<size_t>("n_frames");
	t_start = settings->get<float_p>("t_start");
	t_end = settings->get<float_p>("t_end");

	if (n_frames == 0) {
		std::cerr << "Must have a non-zero, integer number of frames! Exiting." << endl;
		exit(-1);
	}

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

TWMCSystemData::~TWMCSystemData()
{
    delete U;
    delete F;
    delete omega;
    delete E;
}

vector<float_p> TWMCSystemData::GetStoredTimes() const
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


vector<vector<float_p>> TWMCSystemData::GetStoredVariableEvolution(const NoisyMatrix* mat) const
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