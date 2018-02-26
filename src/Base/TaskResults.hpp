//
//  TaskResults.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TaskResults_hpp
#define TaskResults_hpp

#include "Libraries/TStaticFactory.h"

#include "TaskData.hpp"

#include <string>
#include <vector>
#include <cstdio>

#ifdef MPI_SUPPORT
#include "../Libraries/eigen_boost_serialization.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#endif


class TaskResults : public TaskData
{
public:
    explicit TaskResults( size_t nOfDatasets = 1);

    size_t GetId() const;
    void SetId(size_t);

    size_t NumberOfDataSets() const;
    const std::vector<std::string>& NamesOfDatasets() const;
    const std::string NameOfDataset(size_t datasetId) const;
    size_t DataSetSize(size_t id) const;
    size_t ElementsInDataSet(size_t id) const;
    size_t DataSetDimension(size_t id)const;
    unsigned char DataSetDataType(size_t id) const;
    const std::vector<size_t>& DataSetsDimensionData() const;
    void* GetDataSet(size_t id);

    void AddResult(std::string name, void* memAddr, size_t byteSize, size_t elSize,
                   size_t format, size_t dimensions, const size_t* dimData);

    void AddResult(std::string name, void* memAddr, size_t byteSize, size_t elSize,
                   size_t format, std::vector<size_t> dimensions);

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializeExtraData(void* data, unsigned int length);

    // Properties
    float computation_speed = 0;


protected:
    size_t numberOfDatasets = 0;
    std::vector<std::string> namesOfDatasets;
    size_t id = 0;

private:

    // Datasets, and related informations.
    std::vector<void*> datasets; /*WARNING: Should not be serialized*/
    std::vector<size_t> datasetByteSizes;
    std::vector<size_t> datasetElementSize;
    std::vector<size_t> datasetFormat;

    // Dimensionality data. Required when writing register files.
    std::vector<size_t> dimensionsOfDatasets;
    std::vector<size_t> dimensionalityData;

#ifdef MPI_SUPPORT
    friend class boost::serialization::access;
    template<class Archive> void save(Archive & ar, const unsigned int version)
    {
        ar & computation_speed;
        ar & numberOfDatasets;
        ar & namesOfDatasets;
        ar & id;
        ar & datasetByteSizes;
        ar & datasetElementSize;
        ar & datasetFormat;
        ar & dimensionsOfDatasets;
        ar & dimensionalityData;
        ar & datasets;
    }

    template<class Archive> void load(Archive & ar, const unsigned int version)
    {
        ar & computation_speed;
        ar & numberOfDatasets;
        ar & namesOfDatasets;
        ar & id;
        ar & datasetByteSizes;
        ar & datasetElementSize;
        ar & datasetFormat;
        ar & dimensionsOfDatasets;
        ar & dimensionalityData;
        ar & datasets;
    }

    template<class Archive>
    void serializeLocalData(Archive & ar, const unsigned int version)
    {
        ar & computation_speed;
        ar & numberOfDatasets;
        ar & namesOfDatasets;
        ar & id;
        ar & datasetByteSizes;
        ar & datasetElementSize;
        ar & datasetFormat;
        ar & dimensionsOfDatasets;
        ar & dimensionalityData;
        ar & datasets;
    }

#endif
};

#ifdef MPI_SUPPORT
BOOST_CLASS_EXPORT_KEY2(TaskResults, "TaskResults")
#endif

typedef Base::TFactory<std::string, TaskResults, const size_t, const std::string*> ResultsFactory;

#endif /* TaskResults_hpp */
