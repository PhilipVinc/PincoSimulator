//
//  TWMCResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCResults.hpp"

#include "Base/NoisyMatrix.hpp"
#include "TWMCTaskData.hpp"

// First instantiate the allVars type required to hold information about types.
template<>
std::map<TWMCData, std::type_index> allVars<TWMCData>::varTypes = std::map<TWMCData, std::type_index>();
template<>
std::map<TWMCData, const std::string> allVars<TWMCData>::varNames = std::map<TWMCData, const std::string>();

// Then declare all possible variables
static const variable<TWMCData, std::vector<complex_p>> vTraj(TWMCData::traj, "traj");
static const variable<TWMCData, std::vector<float_p>> vWork(TWMCData::work, "work");
static const variable<TWMCData, std::vector<float_p>> vArea(TWMCData::area, "area");
static const variable<TWMCData, MatrixCXd> vUNoise(TWMCData::U_Noise, "U_Realization");
static const variable<TWMCData, MatrixCXd> vFNoise(TWMCData::F_Noise, "F_Realization");
static const variable<TWMCData, MatrixCXd> vDNoise(TWMCData::Delta_Noise, "Omega_Realization");

// Then declare the serialization format of those variables
template<>
const size_t saveFormat<std::vector<complex_p>>::format = 22;
template<>
const size_t saveFormat<std::vector<float_p >>::format = 11;
template<>
const size_t saveFormat<MatrixCXd>::format = 22;


// Serialization stuff

template<>
const unsigned int TWMCResults::SerializingExtraDataOffset()const
{
    return sizeof(extraDataMemory);
}

template<>
const void* TWMCResults::SerializeExtraData()const
{
    return &extraDataMemory;
}

template<>
void TWMCResults::DeSerializeExtraData(void* data, unsigned int length)
{
    // TODO must write
    return;
}

// Trick to force linkage
int dummy2::bogus2::bogus_method() { return 0;}
