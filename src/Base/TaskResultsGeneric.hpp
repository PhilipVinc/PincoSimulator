//
// Created by Filippo Vicentini on 12/03/2018.
//

#ifndef SIMULATOR_GENERICTASKRESULTS_HPP
#define SIMULATOR_GENERICTASKRESULTS_HPP


#include "Base/TaskResults.hpp"
#include "ResultsHelpers/HeterogeneousContainer.hpp"
#include "Base/Utils/Misc.hpp"

#include <array>
#include <stdio.h>
#include <map>
#include <tuple>
#include <vector>
#include <complex>

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

    typedef HeterogeneousContainer<enumType, Types...> containerT;
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

    // This version of AddDataset is used when adding a dataset from rawData.
    // For example, this is used when reading data from a file.
    void AddDataset(std::string name, std::tuple<rawT1, rawT2> data, size_t frames, const std::vector<size_t> dimensions) override {
        // Find the enum # from it's string serialization, then add the dataset.
        enumType t = allVars<enumType>::varEnums.at(name);
        datasets.setUnknownType(t, data, frames, dimensions);

        AddResult(frames, allVars<enumType>::varFormats.at(t), dimensions);
        datasetsTypes.push_back(t);
        datasetsIds.push_back(datasetsIds.size());
    }


    template<typename T>
    T& GetDataset(enumType name) {
        return datasets.template get<T>(name); // TODO What the Fuck is this??
    }

    const std::string DatasetName(enumType eName) const {
        return allVars<enumType>::varNames[eName];
    }

    const std::string DatasetName(size_t datasetId) const override {
        return DatasetName(datasetsTypes[datasetId]);
    }

    inline enumType GetDatasetTypeFromId(int datasetId) const {
        return datasetsTypes[datasetId];
    }

    inline int GetDatasetIdFromType(enumType datasetType) {
        return std::find(datasetsTypes.begin(), datasetsTypes.end(), datasetType) - datasetsTypes.begin();
    }

    const std::vector<std::string> NamesOfDatasets() const override {
        std::vector<std::string> res;
        for (enumType t : datasetsTypes) {
            const std::string dd = DatasetName(t);
            res.push_back(dd);
        }
        return res;
    }

    const size_t DatasetByteSize(size_t id) const override {
        rawTuple vv = datasets.GetSingleRaw(datasetsTypes[id]);
        return std::get<rawT2>(vv);
;    }

    const void* DatasetGet(size_t id) const override{
        rawTuple vv = datasets.GetSingleRaw(datasetsTypes[id]);
        return std::get<rawT1>(vv);
    }

    void AppendExtraData(extraData & localData, extraData && otherData);

    void AppendResult(std::unique_ptr<TaskResults> rhs) override {
        try {

            auto rhs_casted = dynamic_unique_cast<TaskResultsGeneric<enumType, extraData, Types...>>(std::move(rhs));
            // succesfull cast

            // For every id i in other set
            for (auto oi:rhs_casted->datasetsIds) {
                // get the enum type
                enumType type = rhs_casted->GetDatasetTypeFromId(oi);

                size_t nDatasets = NumberOfDataSets();
                int i = rhs_casted->GetDatasetIdFromType(type);
                if (i == nDatasets) {
                    datasetsTypes.push_back(type);
                    datasetsIds.push_back(datasetsIds.size());
                    AddResult(rhs_casted->DatasetElements(oi),
                                allVars<enumType>::varFormats[type],
                              rhs_casted->DatasetDimensions(oi));
                    // add
                    datasets.AppendKey(type, rhs_casted->datasets);
                } else {
                    // merge object in datasets store
                    if( datasets.AppendKey(type, rhs_casted->datasets))
                        datasetElementSize[i] += rhs_casted->datasetElementSize[oi];
                }
            }

            AppendExtraData(extraDataMemory,
                            std::move(rhs_casted->extraDataMemory));
        }
        catch(const std::bad_cast& e) {
            std::cout << "Error: bad cast" << std::endl;
            return;
        }
    }

    // Serialization and deserialization
    const unsigned int SerializingExtraDataOffset()const override;
    const void* SerializeExtraData()const override;
    void DeSerializeExtraData(void* data, unsigned int length) override;


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
