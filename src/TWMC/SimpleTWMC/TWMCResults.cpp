//
//  TWMCResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCResults.hpp"

#include "NoisyMatrix.hpp"
#include "TWMCSimData.hpp"


TWMCResults::TWMCResults(const TWMCSimData* taskData) : TWMCResults(taskData->nx, taskData->ny, taskData->n_frames) {
	extraDataMemory[0] = taskData->t_start;
	extraDataMemory[1] = taskData->t_end;

	// Determine the additional noisy matrices to be saved
	if (taskData->U->HasNoise()) {
		AddComplexMatrixDataset("U_Realizations", taskData->nx, taskData->ny);
	}
	if (taskData->omega->HasNoise()) {
		AddComplexMatrixDataset("Omega_Realizations", taskData->nx, taskData->ny);
	}
	if (taskData->F->HasNoise()) {
		AddComplexMatrixDataset("F_Realizations", taskData->nx, taskData->ny);
	}

	// It's a thermal simulation.
	if (taskData->F->HasTimeDependence()) {
		AddRealMatrixDataset( "dWork", taskData->nx, taskData->ny, taskData->n_frames);
		AddRealMatrixDataset( "dArea", taskData->nx, taskData->ny, taskData->n_frames);
	}
}

TWMCResults::TWMCResults(size_t _nx, size_t _ny, size_t _frames) : nx(_nx), ny(_ny), nxy(nx*ny), frames(_frames)
{
    nxy = ny*nx;
    beta_t = new complex_p[nxy*frames];

	AddResult("traj", beta_t, nx*ny*frames*sizeof(complex_p), frames, 22, {nx,ny});
}

TWMCResults::~TWMCResults()
{
    delete[] beta_t;

	for (int i = 0; i != complexMatrices.size(); i++)
	{
		delete[] complexMatrices[i];
	}
	for (int i = 0; i != realMatrices.size(); i++)
	{
		delete[] realMatrices[i];
	}
}


// Adding datasets
void TWMCResults::AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames)
{
	// create the array holding the quantity;
	complex_p* mat = new complex_p[nx*ny*frames];
	complexMatrices.push_back(mat);

	// Add it to the underlying storage.
	AddResult(name, mat, nx*ny*sizeof(complex_p), frames, 22, {nx,ny});
}

void TWMCResults::AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames)
{
	// create the array holding the quantity;
	float_p* mat = new float_p[nx*ny*frames];
	realMatrices.push_back(mat);

	// Add it to the underlying storage.
	AddResult(name, mat, nx*ny*sizeof(complex_p), frames, 11, {nx,ny});
}


// Serialization stuff

const unsigned int TWMCResults::SerializingExtraDataOffset()const
{
    return sizeof(extraDataMemory);
}

const void* TWMCResults::SerializeExtraData()const
{
    return &extraDataMemory;
}

void TWMCResults::DeSerializegExtraData(void* data, unsigned int length)
{
    // TODO must write
    return;
}
