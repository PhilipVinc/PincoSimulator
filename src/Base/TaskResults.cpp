//
//  TaskResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TaskResults.hpp"


using namespace std;


TaskResults::TaskResults(const size_t _nOfDatasets) : datasetElementSize(_nOfDatasets),
                                                           datasetFormat(_nOfDatasets),
                                                           dimensionsOfDatasets(_nOfDatasets),
                                                           dimensionalityData(_nOfDatasets)
{
    numberOfDatasets = 0;
}


void TaskResults::AddResult(size_t elSize, size_t format, const std::vector<size_t> dimensions)
{
	numberOfDatasets++;
	datasetElementSize.push_back(elSize);
	datasetFormat.push_back(format);

	dimensionsOfDatasets.push_back(dimensions.size());
	for (size_t dim : dimensions)
	{
		dimensionalityData.push_back(dim);
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

size_t TaskResults::NumberOfDataSets()const {
    return numberOfDatasets;
}


unsigned char TaskResults::DataSetDataType(size_t id) const {
	return datasetFormat[id];
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
//CEREAL_REGISTER_TYPE_WITH_NAME(TaskResults, "TaskResults")
#endif
