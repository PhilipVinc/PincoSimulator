//
// Created by Filippo Vicentini on 27/11/2017.
//

#ifndef SIMULATOR_TWMCLIEBRESULTS_HPP
#define SIMULATOR_TWMCLIEBRESULTS_HPP

#include "TaskResults.hpp"
#include "TWMCTypes.h"

#include <stdio.h>

class TWLiebSimData;


class TWMCLiebResults : public TaskResults
{
public:
    TWMCLiebResults(const TWLiebSimData* TaskData);
    TWMCLiebResults(size_t nx, size_t ny, size_t frames);
    ~TWMCLiebResults();

    complex_p* beta_t;
    std::vector<complex_p*> noiseMatrices;
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



#endif //SIMULATOR_TWMCLIEBRESULTS_HPP
