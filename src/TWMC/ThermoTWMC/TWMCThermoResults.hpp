//
//  TWMCThermoResults.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCThermoResults_hpp
#define TWMCThermoResults_hpp

#include "TaskResults.hpp"
#include "TWMCTypes.h"

#include <stdio.h>

class TWMCSimData;

static const size_t nOfSets= 3;
static const string nameOfSets[3]= {"traj", "dWork", "dArea"};

class TWMCThermoResults : public TaskResults
{
public:
    TWMCThermoResults(const TWMCSimData* TaskData);
    TWMCThermoResults(size_t nx, size_t ny, size_t frames);
    ~TWMCThermoResults();
    
    complex_p* beta_t;
    float_p* work_t;
    float_p* area_t;
    size_t nx;
    size_t ny;
    size_t nxy;
    size_t frames;
    
    double extraDataMemory[2];
    
    virtual size_t ElementsInDataSet(size_t id);
    virtual size_t DataSetSize(size_t id);
    virtual unsigned char DataSetDataType(size_t id);

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializegExtraData(void* data, unsigned int length);
    
    
protected:
    
private:
};

#endif /* TWMCThermoResults_hpp */
