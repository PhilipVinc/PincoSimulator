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
    
    dimensionsOfDatasets = {2,2,2};
    dimensionalityData = {taskData->nx, taskData->ny, taskData->nx, taskData->ny, taskData->nx, taskData->ny};
}

TWMCThermoResults::TWMCThermoResults(size_t _nx, size_t _ny, size_t _frames) :
TaskResults(nOfSets, nameOfSets), nx(_nx), ny(_ny), nxy(nx*ny), frames(_frames)
{
    nxy = ny*nx;
    datasets[0] = beta_t = new complex_p[nxy*frames];
    datasets[1] = work_t = new float_p[nxy*frames];
    datasets[2] = area_t = new float_p[nxy*frames];
    //energy_t = new float_p[nxy*frames];
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


unsigned char TWMCThermoResults::DataSetDataType(size_t id)
{
    switch(id)
    {
        case 0:
            // Complex_p
            return 22;
        case 1:
        case 2:
            return 11;
    }
    return 0;
}

size_t TWMCThermoResults::ElementsInDataSet(size_t id)
{
    return frames;
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
