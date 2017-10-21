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


TWMCResults::TWMCResults(const TWMCSimData* taskData) : TWMCResults(taskData->nx, taskData->ny, taskData->n_frames)
{
    extraDataMemory[0] = taskData->t_start;
    extraDataMemory[1] = taskData->t_end;
    
    dimensionsOfDatasets = {2};
    dimensionalityData = {taskData->nx, taskData->ny};

    // Determine the additional noisy matrices to be saved
    if (taskData->U->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        dimensionsOfDatasets.push_back(2);
        dimensionalityData.push_back(taskData->nx);
        dimensionalityData.push_back(taskData->ny);
        complex_p * noiseU = new complex_p[nxy];
        noiseMatrices.push_back(noiseU);
        AddOptionalResult("U_Realizations", noiseU);
    }
    if (taskData->omega->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        dimensionsOfDatasets.push_back(2);
        dimensionalityData.push_back(taskData->nx);
        dimensionalityData.push_back(taskData->ny);
        complex_p * noiseOmega = new complex_p[nxy];
        noiseMatrices.push_back(noiseOmega);
        AddOptionalResult("Omega_Realizations", noiseOmega);
    }
    if (taskData->F->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        dimensionsOfDatasets.push_back(2);
        dimensionalityData.push_back(taskData->nx);
        dimensionalityData.push_back(taskData->ny);
        complex_p * noiseF = new complex_p[nxy];
        noiseMatrices.push_back(noiseF);
        AddOptionalResult("F_Realizations", noiseF);
    }
}

TWMCResults::TWMCResults(size_t _nx, size_t _ny, size_t _frames) : nx(_nx), ny(_ny), nxy(nx*ny), frames(_frames)
{
    nxy = ny*nx;
    beta_t = new complex_p[nxy*frames];

    datasets[0] = beta_t;
}

TWMCResults::~TWMCResults()
{
    delete[] beta_t;

    for (int i = 0; i != noiseMatrices.size(); i++)
    {
        delete[] noiseMatrices[i];
    }
}

size_t TWMCResults::DataSetSize(size_t id)
{
    switch(id)
    {
        case 0:
            return nxy*frames*sizeof(complex_p);

        default:
            return nxy*sizeof(complex_p);
    }

}

size_t TWMCResults::ElementsInDataSet(size_t id)
{
    switch(id)
    {
        case 0:
            return frames;
        default:
            return 1;
    }
}

unsigned char TWMCResults::DataSetDataType(size_t id)
{
    return 22;
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
