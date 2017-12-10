//
// Created by Filippo Vicentini on 27/11/2017.
//

#include "TWMCLiebResults.hpp"

#include "NoisyMatrix.hpp"
#include "TWLiebSimData.hpp"


TWMCLiebResults::TWMCLiebResults(const TWLiebSimData* taskData) :
		TWMCLiebResults(taskData->nx, taskData->ny, taskData->n_frames)
{
    extraDataMemory[0] = taskData->t_start;
    extraDataMemory[1] = taskData->t_end;

	// Determine the additional noisy matrices to be saved
	if (taskData->U_A->HasNoise() || taskData->U_B->HasNoise() ||
	    taskData->U_C->HasNoise()) {
		AddComplexMatrixDataset("U_Realizations", taskData->nx, taskData->ny);
	}

	if (taskData->E_A->HasNoise() || taskData->E_B->HasNoise() ||
	    taskData->E_C->HasNoise()) {
		AddComplexMatrixDataset("Delta_Realizations", taskData->nx, taskData->ny);
	}

	if (taskData->F_A->HasNoise() || taskData->F_B->HasNoise() ||
	    taskData->F_C->HasNoise()) {
	    AddComplexMatrixDataset("F_Realizations", taskData->nx, taskData->ny);
    }
}

TWMCLiebResults::TWMCLiebResults(size_t _nx, size_t _ny, size_t _frames) :
	nx(_nx), ny(_ny), nxy(_nx*_ny), frames(_frames)
{
    nxy = ny*nx;
    beta_t = new complex_p[nxy*3*frames];

	AddResult("traj", beta_t, nxy*3*frames*sizeof(complex_p), frames, 22, {nx,ny,3});
}

TWMCLiebResults::~TWMCLiebResults()
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
void TWMCLiebResults::AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames)
{
    // create the array holding the quantity;
    complex_p* mat = new complex_p[nx*ny*3*frames];
    complexMatrices.push_back(mat);

    // Add it to the underlying storage.
    AddResult(name, mat, nx*ny*sizeof(complex_p), frames, 22, {nx,ny,3});
}

void TWMCLiebResults::AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames)
{
    // create the array holding the quantity;
    float_p* mat = new float_p[nx*ny*3*frames];
    realMatrices.push_back(mat);

    // Add it to the underlying storage.
    AddResult(name, mat, nx*ny*sizeof(complex_p), frames, 11, {nx,ny,3});
}


// Serialization stuff

const unsigned int TWMCLiebResults::SerializingExtraDataOffset()const
{
    return sizeof(extraDataMemory);
}

const void* TWMCLiebResults::SerializeExtraData()const
{
    return &extraDataMemory;
}

void TWMCLiebResults::DeSerializegExtraData(void* data, unsigned int length)
{
    // TODO must write
    return;
}
