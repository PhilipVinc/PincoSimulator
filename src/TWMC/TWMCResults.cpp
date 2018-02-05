//
//  TWMCResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCResults.hpp"

#include "Base/NoisyMatrix.hpp"
#include "TWMCTaskData.hpp"


TWMCResults::TWMCResults(const TWMCTaskData* _taskData) :
		TWMCResults(_taskData->systemData->nx, _taskData->systemData->ny,
		            _taskData->systemData->n_frames, _taskData->systemData->cellSz) {
	TWMCSystemData* taskData = _taskData->systemData;

	extraDataMemory[0] = taskData->t_start;
	extraDataMemory[1] = taskData->t_end;

	// Determine the additional noisy matrices to be saved
	if (taskData->U->HasNoise()) {
		AddComplexMatrixDataset("U_Realizations", taskData->nx, taskData->ny, 1, cellSz);
	}
	if (taskData->omega->HasNoise()) {
		AddComplexMatrixDataset("Omega_Realizations", taskData->nx, taskData->ny, 1, cellSz);
	}
	if (taskData->F->HasNoise()) {
		AddComplexMatrixDataset("F_Realizations", taskData->nx, taskData->ny, 1, cellSz);
	}

	// It's a thermal simulation.
	if (taskData->F->HasTimeDependence()) {
		AddRealMatrixDataset( "dWork", taskData->nx, taskData->ny, taskData->n_frames, cellSz);
		AddRealMatrixDataset( "dArea", taskData->nx, taskData->ny, taskData->n_frames, cellSz);
		work_t = * (realMatrices[0]);
		area_t = * (realMatrices[1]);
	}
}

TWMCResults::TWMCResults(size_t _nx, size_t _ny, size_t _frames, size_t _cellSz = 1)
		: nx(_nx), ny(_ny), nxy(nx * ny), frames(_frames), cellSz(_cellSz),
          beta_t(_nx*_ny*_cellSz*_frames)
{
    nxy = ny*nx;

	AddResult("traj", beta_t.data(), nx*ny*cellSz*frames*sizeof(complex_p), frames, 22, {nx,ny, _cellSz});
}

TWMCResults::~TWMCResults()
{
	//cout << "Deleting TWMCResult with ID: " << id << endl;
    //delete[] beta_t;
	for (int i = 0; i != complexMatrices.size(); i++)
	{
        delete complexMatrices[i];
	}
	for (int i = 0; i != realMatrices.size(); i++)
	{
        delete realMatrices[i];
	}
}


// Adding datasets
void TWMCResults::AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames,
                                          size_t cellSz)
{
	// create the array holding the quantity;
	//complex_p* mat = new complex_p[nx*ny*cellSz*frames];
    std::vector<complex_p>* mat = new std::vector<complex_p>(nx*ny*cellSz*frames);

    complexMatrices.push_back(mat);

	// Add it to the underlying storage.
	AddResult(name, &mat->at(0), nx*ny*cellSz*sizeof(complex_p), frames, 22, {nx,ny, cellSz});
}

void TWMCResults::AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames,
                                       size_t cellSz)
{
	// create the array holding the quantity;
	//float_p* mat = new float_p[nx*ny*cellSz*frames];
    std::vector<float_p>* mat = new std::vector<float_p>(nx*ny*cellSz*frames);
	realMatrices.push_back(mat);

	// Add it to the underlying storage.
	AddResult(name, &mat->at(0), nx*ny*cellSz*sizeof(float_p), frames, 11, {nx,ny, cellSz});
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