//
//  TWMCThermoResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCThermoResults.hpp"
#include "TWMCSimData.hpp"


TWMCThermoResults::TWMCThermoResults(const TWMCSimData* taskData) :
    TWMCThermoResults(taskData->nx, taskData->ny, taskData->n_frames)
{
    extraDataMemory[0] = taskData->t_start;
    extraDataMemory[1] = taskData->t_end;
    
    dimensionsOfDatasets = {0,0,0};
    dimensionalityData = {};
}

TWMCThermoResults::TWMCThermoResults(size_t _nx, size_t _ny, size_t _frames) :
TaskResults(nOfSets, nameOfSets), nx(_nx), ny(_ny), nxy(nx*ny), frames(_frames)
{
    nxy = ny*nx;
    beta_t = new complex_p[nxy*frames];
    work_t = new float_p[nxy*frames];
    area_t = new float_p[nxy*frames];
}

TWMCThermoResults::~TWMCThermoResults()
{
    delete[] beta_t;
}

size_t TWMCThermoResults::DataSetSize(size_t id)
{
    switch(id)
    {
        case 0:
            return nxy*frames*sizeof(complex_p);
            
        case 1:
        case 2:
            return nxy*frames*sizeof(float_p);
    }
    return 0;
}

size_t TWMCThermoResults::ElementsInDataSet(size_t id)
{
    return frames;
}

void* TWMCThermoResults::GetDataSet(size_t id)
{
    switch (id) {
        case 0:
            return beta_t;
            break;
        case 1:
            return work_t;
            break;
        case 2:
            return area_t;
            break;

        default:
            break;
    }
    return nullptr;
}


// Serialization stuff

const unsigned int TWMCThermoResults::SerializingExtraDataOffset()const
{
    return sizeof(extraDataMemory);
}

const void* TWMCThermoResults::SerializeExtraData()const
{
    return &extraDataMemory;
}

void TWMCThermoResults::DeSerializegExtraData(void* data, unsigned int length)
{
    // must write
    return;
}
