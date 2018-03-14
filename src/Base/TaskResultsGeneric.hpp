//
// Created by Filippo Vicentini on 12/03/2018.
//

#ifndef SIMULATOR_GENERICTASKRESULTS_HPP
#define SIMULATOR_GENERICTASKRESULTS_HPP


#include "Base/TaskResults.hpp"
#include "ResultsHelpers/HeterogeneousContainer.hpp"

#include <array>
#include <stdio.h>
#include <map>
#include <tuple>

#ifdef MPI_SUPPORT
#include "Base/Serialization/SerializationArchiveFormats.hpp"
#include <Libraries/eigen_cereal_serialization.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#endif

template <typename enumType, typename extraData, typename... Types>
class TaskResultsGeneric :public TaskResults {
public:
    //TaskResultsGeneric() {};
    //~TaskResultsGeneric() {};

    typedef MyClass<enumType, Types...> containerT;
    containerT datasets;
    std::vector<enumType> datasetsTypes;
    std::vector<int> datasetsIds;
    extraData extraDataMemory;

    template<typename T>
    void AddDataset(enumType name, T& data, size_t frames, const std::vector<size_t>&& dimensions) {
        datasets.set(name, data);

        AddResult(frames, saveFormat<T>::format, dimensions);
        datasetsTypes.push_back(name);
        datasetsIds.push_back(datasetsIds.size());
    }

    template<typename T>
    void AddDataset(enumType name, T&& data, size_t frames, const std::vector<size_t>&& dimensions) {
        datasets.set(name, data);

        AddResult(frames, saveFormat<T>::format, dimensions);
        datasetsTypes.push_back(name);
        datasetsIds.push_back(datasetsIds.size());
    }

    virtual void AddDataset(std::string name, std::tuple<rawT1, rawT2> data, size_t frames, const std::vector<size_t> dimensions) {

        enumType t = allVars<enumType>::varEnums.at(name);

        datasets.setUnknownType(t, data, frames, dimensions);


        AddResult(frames, allVars<enumType>::varFormats.at(t), dimensions);
        datasetsTypes.push_back(t);
        datasetsIds.push_back(datasetsIds.size());
    }


    template<typename T>
    T& GetDataset(enumType name) {
        datasets.get(name);
    }

    const std::string DatasetName(enumType eName) const {
        return allVars<enumType>::varNames[eName];
    }

    virtual const std::string NameOfDataset(size_t datasetId) const {
        return DatasetName(datasetsTypes[datasetId]);
    }

    virtual const std::vector<std::string> NamesOfDatasets() const {
        std::vector<std::string> res;
        for (enumType t : datasetsTypes) {
            const std::string dd = DatasetName(t);
            res.push_back(dd);
        }
        return res;
    }

    virtual const size_t DataSetSize(size_t id) const {
        rawTuple vv = datasets.GetSingleRaw(datasetsTypes[id]);
        return std::get<rawT2>(vv);
;    }

    virtual const void* GetDataSet(size_t id) const {
        rawTuple vv = datasets.GetSingleRaw(datasetsTypes[id]);
        return std::get<rawT1>(vv);
    }

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializeExtraData(void* data, unsigned int length);


#ifdef MPI_SUPPORT
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::virtual_base_class<TaskResults>(this));
        ar(datasetsIds);
        ar(datasetsTypes);
        ar(extraDataMemory);
        ar(datasets);
    };
#endif
};


#endif //SIMULATOR_GENERICTASKRESULTS_HPP
