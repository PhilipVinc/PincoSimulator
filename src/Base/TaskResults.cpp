//
//  TaskResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TaskResults.hpp"


using namespace std;


TaskResults::TaskResults(const size_t _nOfDatasets,
                         const string* _namesOfDatasets) : datasets(_nOfDatasets)
{
    numberOfDatasets = _nOfDatasets;
    namesOfDatasets.reserve(_nOfDatasets+3);
    for (int i = 0; i != _nOfDatasets; i++)
    {
        namesOfDatasets.push_back(_namesOfDatasets[i]);
    }
}

void TaskResults::AddOptionalResult(const string name, void* memAddr)
{
    numberOfDatasets++;
    namesOfDatasets.push_back(name);
    datasets.push_back(memAddr);
}


size_t TaskResults::GetId()
{
    return id;
}

void TaskResults::SetId(size_t _id)
{
    id = _id;
}

void* TaskResults::GetDataSet(size_t id)
{
    return datasets[id];
}

const size_t TaskResults::NumberOfDataSets()const
{
    return numberOfDatasets;
}

const string TaskResults::NameOfDataset(const size_t datasetId)
{
    return namesOfDatasets[datasetId];
}

const vector<string>& TaskResults::NamesOfDatasets()const
{
    return namesOfDatasets;
}

const size_t TaskResults::DataSetDimension(size_t id)const
{
    return dimensionsOfDatasets[id];
}

const vector<size_t>& TaskResults::DataSetsDimensionData()const
{
    return dimensionalityData;
}

const unsigned int TaskResults::SerializingExtraDataOffset()const
{
    return 0;
}

const void* TaskResults::SerializeExtraData()const
{
    return nullptr;
}

void TaskResults::DeSerializegExtraData(void* data, unsigned int length)
{
    return;
}
