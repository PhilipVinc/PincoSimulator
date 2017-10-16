//
//  TaskResults.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TaskResults_hpp
#define TaskResults_hpp

#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

static string default_dataset_name = "traj";

class TaskResults
{
public:
    TaskResults(const size_t nOfDatasets = 1,
                const string* namesOfDatasets = &default_dataset_name);
    
    virtual size_t DataSetSize(size_t id) = 0;
    virtual size_t ElementsInDataSet(size_t id) = 0;
    const size_t DataSetDimension(size_t id)const;
    const vector<size_t>& DataSetsDimensionData()const;
    void* GetDataSet(size_t id);
    virtual unsigned char DataSetDataType(size_t id) = 0;

    void AddOptionalResult(const string name, void* memAddr);

    void SetId(size_t);
    size_t GetId();
    const size_t NumberOfDataSets()const;
    const string NameOfDataset(const size_t datasetId);
    const vector<string>& NamesOfDatasets()const;
    
    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializegExtraData(void* data, unsigned int length);
    
    float computation_speed = 0;

protected:
    size_t numberOfDatasets = 0;
    vector<string> namesOfDatasets;
    vector<size_t> dimensionsOfDatasets;
    vector<size_t> dimensionalityData;

    vector<void*> datasets;
private:
    size_t id = 0;
    
};

#endif /* TaskResults_hpp */
