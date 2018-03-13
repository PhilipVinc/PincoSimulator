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
#include "Base/Serialization/SerializationArchiveFormats.hpp"
#include "../Libraries/eigen_cereal_serialization.hpp"
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/access.hpp>
#endif


class TaskResults : public TaskData
{
public:
    explicit TaskResults( size_t nOfDatasets = 1);

    size_t GetId() const;
    void SetId(size_t);

    // Accessing the datasets
    size_t NumberOfDataSets() const;
    size_t ElementsInDataSet(size_t id) const;
    size_t DataSetDimension(size_t id)const;
    unsigned char DataSetDataType(size_t id) const;
    const std::vector<size_t>& DataSetsDimensionData() const;

    // Pure virtual methods
    virtual const std::vector<std::string> NamesOfDatasets() const = 0;
    virtual const std::string NameOfDataset(size_t datasetId) const = 0 ;
    virtual const size_t DataSetSize(size_t id) const = 0;
    virtual const void* GetDataSet(size_t id) const = 0;

    void AddResult(size_t elSize, size_t format, const std::vector<size_t> dimensions);

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializeExtraData(void* data, unsigned int length);

    // Properties
    float computation_speed = 0;


protected:
    size_t numberOfDatasets = 0;
    size_t id = 0;

private:

    // Datasets, and related informations.
    std::vector<size_t> datasetElementSize;
    std::vector<size_t> datasetFormat;

    // Dimensionality data. Required when writing register files.
    std::vector<size_t> dimensionsOfDatasets;
    std::vector<size_t> dimensionalityData;

#ifdef MPI_SUPPORT
public:
    template<class Archive> void save(Archive & ar) const
    {
        ar(computation_speed);
        ar(numberOfDatasets);
        ar(id);
        ar(datasetElementSize);
        ar(datasetFormat);
        ar(dimensionsOfDatasets);
        ar(dimensionalityData);
    }

    template<class Archive> void load(Archive & ar)
    {
        ar(computation_speed);
        ar(numberOfDatasets);
        ar(id);
        ar(datasetElementSize);
        ar(datasetFormat);
        ar(dimensionsOfDatasets);
        ar(dimensionalityData);
    }
#endif
};

#ifdef MPI_SUPPORT
namespace cereal {
    template <class Archive>
    struct specialize<Archive, TaskResults, cereal::specialization::member_load_save> {};
} // namespace ...
#endif

typedef Base::TFactory<std::string, TaskResults, const size_t, const std::string*> ResultsFactory;

#endif /* TaskResults_hpp */
