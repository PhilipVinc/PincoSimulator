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


TWMCResults::TWMCResults(const TWMCTaskData* _taskData) :
		TWMCResults(_taskData->systemData->nx, _taskData->systemData->ny,
		            _taskData->systemData->n_frames, _taskData->systemData->cellSz) {
	TWMCSystemData* taskData = _taskData->systemData.get();

	extraDataMemory[0] = taskData->t_start;
	extraDataMemory[1] = taskData->t_end;

	// It's a thermal simulation.
	if (taskData->F->HasTimeDependence()) {
		AddRealMatrixDataset(variables::work, taskData->nx, taskData->ny, taskData->n_frames, cellSz);
		AddRealMatrixDataset(variables::area, taskData->nx, taskData->ny, taskData->n_frames, cellSz);
	}
}

TWMCResults::TWMCResults()
{

}

TWMCResults::TWMCResults(size_t _nx, size_t _ny, size_t _frames, size_t _cellSz = 1)
		: nx(_nx), ny(_ny), nxy(nx * ny), frames(_frames), cellSz(_cellSz)
{
}

TWMCResults::~TWMCResults()
{

}


// Adding datasets
void TWMCResults::AddComplexMatrixDataset(variables var, std::vector<complex_p> data,
                                          size_t frames, const std::vector<size_t> dimensions)
{
    // create the array holding the quantity;
    complexMatrices.push_back(data);

    // Add it to the underlying storage.
    datasetList.push_back(var);
    datasetIndex.insert(std::pair<variables,std::tuple<varType, int>>(var, std::make_tuple(varType::Complex,
                                                                                           complexMatrices.size()-1)));

    AddResult(nx*ny*cellSz*frames*sizeof(complex_p), frames, 22, dimensions);
}

void TWMCResults::AddRealMatrixDataset(variables var, std::vector<float_p> data,
                                       size_t frames, const std::vector<size_t> dimensions)
{
    // create the array holding the quantity;
    realMatrices.push_back(data);

    // Add it to the underlying storage.
    datasetList.push_back(var);
    datasetIndex.insert(std::pair<variables,std::tuple<varType, int>>(var, std::make_tuple(varType::Real,
                                                                                           realMatrices.size()-1)));

    AddResult(nx*ny*cellSz*frames*sizeof(float_p), frames, 22, dimensions);
}

void TWMCResults::AddComplexMatrixDataset(variables var, size_t nx, size_t ny, size_t frames,
                                          size_t cellSz)
{
	// create the array holding the quantity;
	complexMatrices.emplace_back(nx*ny*cellSz*frames);

	// Add it to the underlying storage.
	datasetList.push_back(var);
	datasetIndex.insert(std::pair<variables,std::tuple<varType, int>>(var, std::make_tuple(varType::Complex,
	                                                                                       complexMatrices.size()-1)));

	AddResult(nx*ny*cellSz*frames*sizeof(complex_p), frames, 22, {nx,ny, cellSz});
}

void TWMCResults::AddRealMatrixDataset(variables var, size_t nx, size_t ny, size_t frames,
                                          size_t cellSz)
{
    // create the array holding the quantity;
    realMatrices.emplace_back(nx*ny*cellSz*frames);

    // Add it to the underlying storage.
    datasetList.push_back(var);
	datasetIndex.insert(std::pair<variables,std::tuple<varType, int>>(var, std::make_tuple(varType::Real,
	                                                                                       realMatrices.size()-1)));

    AddResult(nx*ny*cellSz*frames*sizeof(float_p), frames, 22, {nx,ny, cellSz});
}

float_p* TWMCResults::GetRealDataset(variables var) {
	std::tuple<varType, int> loc = datasetIndex.at(var);
	int ss = std::get<1>(loc);
	return &realMatrices[ss][0];
}

complex_p* TWMCResults::GetComplexDataset(variables var) {
	std::tuple<varType, int> loc = datasetIndex.at(var);
	int ss = std::get<1>(loc);
	return &complexMatrices[ss][0];
}

const void* TWMCResults::GetDataSet(size_t datasetId) const {
    std::tuple<varType, int> loc = datasetIndex.at(datasetList[datasetId]);
	varType tt = std::get<0>(loc);
	int ss = std::get<1>(loc);
	switch(tt) {
		case Real:
			return &realMatrices[ss][0];
		case Complex:
			return &complexMatrices[ss][0];
	}
}

const std::string TWMCResults::NameOfDataset(size_t datasetId) const
{
    auto varT = datasetList[datasetId];
	return NameOfDataset(varT);
}

const std::string TWMCResults::NameOfDataset(variables varT) const
{
	switch (varT) {
		case traj:
			return "traj";
		case work:
			return "dWork";
		case area:
			return "dArea";
		case U_Noise:
			return "U_Realizations";
		case F_Noise:
			return "F_Realizations";
		case J_Noise:
			return "J_Realizations";
		case Delta_Noise:
			return "Delta_Realizations";
	}
}

const std::vector<std::string> TWMCResults::NamesOfDatasets() const
{
	std::vector<std::string> names(datasetList.size()); int i=0;
	for (variables var : datasetList) {
		names[i] = NameOfDataset(var); i++;
	}
	return names;
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