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
    float_p* work_t;
    float_p* area_t;
	std::vector<complex_p*> complexMatrices;
	std::vector<float_p*> realMatrices;
	size_t nx;
    size_t ny;
    size_t nxy;
    size_t frames;

	void AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames = 1);
	void AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames = 1);

    // Serialization and deserialization
    double extraDataMemory[2];
	const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializegExtraData(void* data, unsigned int length);


protected:

private:

};



#endif //SIMULATOR_TWMCLIEBRESULTS_HPP
