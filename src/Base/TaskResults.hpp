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
    explicit TaskResults( const size_t nOfDatasets = 1);
    inline size_t GetId() const { return id;};
    inline void SetId(size_t _id) { id = _id;};

    // Accessing the datasets
    inline size_t NumberOfDataSets() const {return numberOfDatasets;};
    inline const std::vector<size_t>& DataSetsDimensionData() const {return dimensionalityData;};
    virtual const std::vector<std::string> NamesOfDatasets() const = 0;

    // Dataset-specific properties
    inline size_t DatasetElements(size_t id) const {return datasetElementSize[id];};
    inline size_t DatasetDimensionsN(size_t id)const {return dimensionsOfDatasets[id];};
    std::vector<size_t> DatasetDimensions(size_t id) const;
    inline unsigned char DatasetFormat(size_t id) const {return datasetFormat[id];};
    virtual const std::string DatasetName(size_t datasetId) const = 0 ; // pv
    virtual const size_t DatasetByteSize(size_t id) const = 0;          // pv
    virtual const void* DatasetGet(size_t id) const = 0;                // pv

    virtual void AddDataset(std::string name, std::tuple<const void*, size_t> data,
                    size_t frames, const std::vector<size_t> dimensions) = 0;

    virtual void AppendResult(std::unique_ptr<TaskResults> rhs) = 0;

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializeExtraData(void* data, unsigned int length);

    // Properties
    float computation_speed = 0;


protected:
    size_t numberOfDatasets = 0;
    size_t id = 0;
    std::vector<size_t> datasetElementSize;

    void AddResult(size_t elSize, size_t format, std::vector<size_t> dimensions);

private:

    // Datasets, and related informations.
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

typedef Base::TFactory<std::string, TaskResults> ResultsFactory;

#endif /* TaskResults_hpp */
