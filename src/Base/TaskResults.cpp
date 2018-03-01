//
//  TaskResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TaskResults.hpp"


using namespace std;


TaskResults::TaskResults(const size_t _nOfDatasets) : datasets(0),
                                                           datasetByteSizes(0),
                                                           datasetElementSize(0),
                                                           datasetFormat(0),
                                                           dimensionsOfDatasets(0),
                                                           dimensionalityData(0)
{
    numberOfDatasets = 0;
}


void TaskResults::AddResult(const std::string name, void* memAddr, size_t byteSize, size_t elSize,
                            size_t format, const std::vector<size_t> dimensions)
{
	AddResult(name, memAddr, byteSize, elSize, format, dimensions.size(), dimensions.data());
}

void TaskResults::AddResult(const std::string name, void* memAddr, size_t byteSize, size_t elSize,
                            size_t format, size_t dimensions, const size_t* dimData)
{
	numberOfDatasets++;
	namesOfDatasets.push_back(name);
	datasets.push_back(memAddr);
	datasetByteSizes.push_back(byteSize);
	datasetElementSize.push_back(elSize);
	datasetFormat.push_back(format);

	dimensionsOfDatasets.push_back(dimensions);
	for (size_t i = 0; i!= dimensions; ++i)
	{
		dimensionalityData.push_back(dimData[i]);
	}
}



// --------------------
// Generic Accessor Methods

size_t TaskResults::GetId() const{
    return id;
}

void TaskResults::SetId(size_t _id) {
    id = _id;
}

size_t TaskResults::ElementsInDataSet(size_t id) const {
	return datasetElementSize[id];
}

size_t TaskResults::DataSetSize(size_t id) const {
	return datasetByteSizes[id];
}

void* TaskResults::GetDataSet(size_t id) {
    return datasets[id];
}

size_t TaskResults::NumberOfDataSets()const {
    return numberOfDatasets;
}

const string TaskResults::NameOfDataset(const size_t datasetId)const {
    return namesOfDatasets[datasetId];
}

unsigned char TaskResults::DataSetDataType(size_t id) const {
	return datasetFormat[id];
}

const vector<string>& TaskResults::NamesOfDatasets()const {
    return namesOfDatasets;
}

size_t TaskResults::DataSetDimension(size_t id)const {
    return dimensionsOfDatasets[id];
}

const vector<size_t>& TaskResults::DataSetsDimensionData()const {
    return dimensionalityData;
}


// ------------------ //
// Serialization and default null methods (to take care when
// the libraries do not implement those. They are virtual anyawy

const unsigned int TaskResults::SerializingExtraDataOffset()const {
    return 0;
}

const void* TaskResults::SerializeExtraData()const {
    return nullptr;
}

void TaskResults::DeSerializeExtraData(void* data, unsigned int length) {
    return;
}

// ------------------ //
// Boost::Serialization
#ifdef MPI_SUPPORT
BOOST_CLASS_EXPORT_IMPLEMENT(TaskResults)
#endif
