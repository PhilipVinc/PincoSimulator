//
// Created by Filippo Vicentini on 27/11/2017.
//

#include "TWMCLiebResults.hpp"

#include "NoisyMatrix.hpp"
#include "TWLiebSimData.hpp"


TWMCLiebResults::TWMCLiebResults(const TWLiebSimData* taskData) : TWMCLiebResults(taskData->nx, taskData->ny, taskData->n_frames)
{
    extraDataMemory[0] = taskData->t_start;
    extraDataMemory[1] = taskData->t_end;

    dimensionsOfDatasets = {3};
    dimensionalityData = {taskData->nx, taskData->ny, 3};

    // Determine the additional noisy matrices to be saved is U
    if (taskData->U_A->GetNoiseType() != NoisyMatrix::NoiseType::None &&
            taskData->U_B->GetNoiseType() != NoisyMatrix::NoiseType::None &&
            taskData->U_C->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        dimensionsOfDatasets.push_back(3);
        dimensionalityData.push_back(taskData->nx);
        dimensionalityData.push_back(taskData->ny);
        dimensionalityData.push_back(3);
        complex_p * noiseU = new complex_p[nxy*3];
        noiseMatrices.push_back(noiseU);
        AddOptionalResult("U_Realizations", noiseU);
    }


    if (taskData->E_A->GetNoiseType() != NoisyMatrix::NoiseType::None &&
            taskData->E_B->GetNoiseType() != NoisyMatrix::NoiseType::None &&
            taskData->E_C->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        dimensionsOfDatasets.push_back(3);
        dimensionalityData.push_back(taskData->nx);
        dimensionalityData.push_back(taskData->ny);
        dimensionalityData.push_back(3);
        complex_p * noiseOmega = new complex_p[nxy];
        noiseMatrices.push_back(noiseOmega);
        AddOptionalResult("Delta_Realizations", noiseOmega);
    }

    if (taskData->F_A->GetNoiseType() != NoisyMatrix::NoiseType::None &&
            taskData->F_B->GetNoiseType() != NoisyMatrix::NoiseType::None &&
            taskData->F_C->GetNoiseType() != NoisyMatrix::NoiseType::None)
    {
        dimensionsOfDatasets.push_back(3);
        dimensionalityData.push_back(taskData->nx);
        dimensionalityData.push_back(taskData->ny);
        dimensionalityData.push_back(3);
        complex_p * noiseF = new complex_p[nxy*3];
        noiseMatrices.push_back(noiseF);
        AddOptionalResult("F_Realizations", noiseF);
    }
}

TWMCLiebResults::TWMCLiebResults(size_t _nx, size_t _ny, size_t _frames) : nx(_nx), ny(_ny), nxy(nx*ny), frames(_frames)
{
    nxy = ny*nx;
    beta_t = new complex_p[nxy*3*frames];

    datasets[0] = beta_t;
}

TWMCLiebResults::~TWMCLiebResults()
{
    delete[] beta_t;

    for (int i = 0; i != noiseMatrices.size(); i++)
    {
        delete[] noiseMatrices[i];
    }
}

size_t TWMCLiebResults::DataSetSize(size_t id)
{
    switch(id)
    {
        case 0:
            return nxy*3*frames*sizeof(complex_p);

        default:
            return nxy*3*sizeof(complex_p);
    }

}

size_t TWMCLiebResults::ElementsInDataSet(size_t id)
{
    switch(id)
    {
        case 0:
            return frames;
        default:
            return 1;
    }
}

unsigned char TWMCLiebResults::DataSetDataType(size_t id)
{
    return 22;
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
