//
//  TWMCResults.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCResults_hpp
#define TWMCResults_hpp

#include "TaskResults.hpp"
#include "TWMCTypes.h"

#include <stdio.h>

class TWMCSimData;


class TWMCResults : public TaskResults
{
public:
    TWMCResults(const TWMCSimData* TaskData);
    TWMCResults(size_t nx, size_t ny, size_t frames);
    ~TWMCResults();
    
    complex_p* beta_t;
    std::vector<complex_p*> complexMatrices;
    std::vector<float_p*> realMatrices;
    size_t nx;
    size_t ny;
    size_t nxy;
    size_t frames;
    
    double extraDataMemory[2];
    void AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames = 1);
    void AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames = 1);

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializegExtraData(void* data, unsigned int length);


protected:


private:


};

#endif /* TWMCResults_hpp */
