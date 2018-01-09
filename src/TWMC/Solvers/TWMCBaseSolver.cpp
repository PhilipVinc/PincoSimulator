//
// Created by Filippo Vicentini on 22/12/17.
//

#include "TWMCBaseSolver.hpp"

#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"
#include "TWMC/TWMCResults.hpp"

#include "Base/NoisyMatrix.hpp"

#include <mutex>

TWMCBaseSolver::TWMCBaseSolver() {
	nTasksToRequest = 1;
}

TWMCBaseSolver::~TWMCBaseSolver() {

}

inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
std::mutex fftw_mutex;

void TWMCBaseSolver::Setup() {
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

std::vector<TaskResults*> TWMCBaseSolver::Compute(const std::vector<TaskData *> tasks)
{
	cout << "computing" <<endl;
	std::vector<TaskResults*> allResults;
	for (size_t i =0; i < tasks.size(); i++ )
	{
		TWMCTaskData* task = static_cast<TWMCTaskData*>(tasks[i]);

		// Check if the system is always the same
		if (task->systemData != lastSharedSystemData)
		{
			lastSharedSystemData = task->systemData;
			Setup();
		}

		// Setup the single simulation
		TWMCResults* res = new TWMCResults(task);
		allResults.push_back(res);
		unsigned int seed = task->rngSeed;
		res->SetId(seed);
		auto initialCondition = TWMCTaskData::InitialConditions::ReadFromSettings;

		std::mt19937 gen(seed);
		std::normal_distribution<> normal(0,1); // mean = 0, std = 1;

		const TWMCSystemData* data = lastSharedSystemData;

		// Generate noisy Matrices
		bool updateMats = false; size_t noiseN = 0;
		if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None)
		{
			updateMats=true;
			U = data->U->Generate(gen);

			// And now store this matrix in the results
			complex_p* resData = res->complexMatrices[noiseN]; noiseN++;
			complex_p* matData = U.data();

			for (unsigned j= 0; j < data->nxy; j++)
			{
				resData[j] = matData[j];
			}
		}
		if (data->omega->GetNoiseType() != NoisyMatrix::NoiseType::None)
		{
			updateMats=true;
			omega = data->omega->Generate(gen);

			// And now store this matrix in the results
			complex_p* resData = res->complexMatrices[noiseN]; noiseN++;
			complex_p* matData = omega.data();

			for (unsigned j= 0; j < data->nxy; j++)
			{
				resData[j] = matData[j];
			}
		}
		if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None)
		{
			updateMats=true;
			F = data->F->Generate(gen);

			// And now store this matrix in the results
			complex_p* resData = res->complexMatrices[noiseN]; noiseN++;
			complex_p* matData = F.data();

			for (unsigned j= 0; j < data->nxy; j++)
			{
				resData[j] = matData[j];
			}
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
		//int frame_steps = floor(data->dt_obs/data->dt);

		// Initialize the beta value to the starting value.

		plan->fft_i_out = beta_t;

		auto dt4 = sqrt(data->gamma_val*data->dt/4.0);
		auto dt = data->dt;

		TWMC_FFTW_plans& _plan = *plan;

		while (t<=t_end)
		{
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
			a_t = ((real_step_linear.array() + ij*U.array()*(beta_t.array().abs2()-1.0) )*beta_t.array() + _plan.fft_i_out.array() + ij*F.array())*dt;
			randCMat(&tmpRand, gen, normal);
			kai_t = beta_t.array() + a_t.array() + dt4*tmpRand.array();

			beta_t = kai_t;

			// Print the data
			if((i_step % data->frame_steps ==0 ) && i_frame < data->n_frames)
			{
				size_t size = res->nx*res->ny;
				complex_p* data = beta_t.data();
				memcpy(&res->beta_t[i_frame*size], data, sizeof(complex_p)*size);

				i_frame = i_frame + 1;
			}
			t += data->dt;
			i_step++;
		}
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

float TWMCBaseSolver::ApproximateComputationProgress() const {
	return float((t-t_start)/t_end);
}