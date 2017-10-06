//
//  TWMCResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCResults.hpp"
#include "TWMCSimData.hpp"


TWMCResults::TWMCResults(const TWMCSimData* taskData) : TWMCResults(taskData->nx, taskData->ny, taskData->n_frames)
{
    extraDataMemory[0] = taskData->t_start;
    extraDataMemory[1] = taskData->t_end;
    
    dimensionsOfDatasets = {0};
    dimensionalityData = {};
}

TWMCResults::TWMCResults(size_t _nx, size_t _ny, size_t _frames) : nx(_nx), ny(_ny), nxy(nx*ny), frames(_frames)
{
    nxy = ny*nx;
    beta_t = new complex_p[nxy*frames];
}

TWMCResults::~TWMCResults()
{
    delete[] beta_t;
}

size_t TWMCResults::DataSetSize(size_t id)
{
    return nxy*frames*sizeof(complex_p);
}

size_t TWMCResults::ElementsInDataSet(size_t id)
{
    return frames;
}

void* TWMCResults::GetDataSet(size_t id)
{
    return beta_t;
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
    // must write
    return;
}
