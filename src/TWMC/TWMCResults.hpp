//
//  TWMCResults.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCResults_hpp
#define TWMCResults_hpp

#include "Base/TaskResults.hpp"
#include "TWMCTypes.h"

#include <array>
#include <stdio.h>
#include <map>
#include <tuple>

class TWMCTaskData;

#ifdef MPI_SUPPORT
#include "Base/MPITaskProcessor/SerializationArchiveFormats.hpp"

#include "Libraries/eigen_cereal_serialization.hpp"
#include <cereal/types/complex.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp> // For LoadAndConstruct
#include <cereal/cereal.hpp>
#endif

enum variables { traj, work, area,
    U_Noise, F_Noise, Delta_Noise, J_Noise};

//auto variableName = {"traj", "dWork", "dArea",
//                    "U_Realizations", "F_Realizations", "Delta_Realizations"};

enum varType {Real, Complex};


class TWMCResults : public TaskResults
{
public:
    TWMCResults();
    TWMCResults(const TWMCTaskData* TaskData);
    TWMCResults(size_t nx, size_t ny, size_t frames, size_t _cellSz);
    ~TWMCResults();

    std::vector<std::vector<complex_p>> complexMatrices;
    std::vector<std::vector<float_p>> realMatrices;
    size_t nx;
    size_t ny;
    size_t cellSz;
    size_t nxy;
    size_t frames;

    std::map<variables, std::tuple<varType, int>> datasetIndex;
    std::vector<variables> datasetList;

    std::array<double, 2> extraDataMemory;
    //void AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames,  size_t cellSz);
    //void AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames,
    //                             size_t cellSz);

    void AddRealMatrixDataset(variables var, size_t nx, size_t ny, size_t frames,  size_t cellSz);
    void AddComplexMatrixDataset(variables var, size_t nx, size_t ny, size_t frames,
                                 size_t cellSz);

    virtual const void* GetDataSet(size_t id) const;
    virtual const std::vector<std::string> NamesOfDatasets() const;
    const std::string NameOfDataset(size_t datasetId) const;
    const std::string NameOfDataset(variables varT) const;
    float_p* GetRealDataset(variables var);
    complex_p* GetComplexDataset(variables var);

    //const virtual GetDataset

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializegExtraData(void* data, unsigned int length);

#ifdef MPI_SUPPORT
    template<class Archive>
    void save(Archive  & ar) const
    {
        ar(cereal::virtual_base_class<TaskResults>(this));

        ar(nx);
        ar(ny);
        ar(frames);
        ar(cellSz);
        ar(frames);
        ar(extraDataMemory);

        ar(complexMatrices);
        ar(realMatrices);
        ar(datasetIndex);
        ar(datasetList);
    };


    template<class Archive>
    void load(Archive  & ar)
    {
        ar(cereal::virtual_base_class<TaskResults>(this));

        ar(nx);
        ar(ny);
        ar(frames);
        ar(cellSz);
        ar(frames);
        ar(extraDataMemory);

        ar(complexMatrices);
        ar(realMatrices);
        ar(datasetIndex);
        ar(datasetList);
    };
#endif

protected:


private:
#ifdef MPI_SUPPORT
    friend class cereal::access;
#endif


};

#ifdef MPI_SUPPORT

// Support for serializing NoisyMatrix, as it has no default constructor
namespace cereal {
    /*template <> struct LoadAndConstruct<TWMCResults>
    {
        // load_and_construct will be passed the archive that you will be loading
        // from as well as a construct object which you can use as if it were the
        // constructor for your type.  cereal will handle all memory management for you.
        template <class Archive>
        static void load_and_construct( Archive & ar, cereal::construct<TWMCResults> & construct )
        {
            size_t nx;	ar(nx);
            size_t ny;  ar(ny);
            size_t frames; ar(frames);
            size_t cellSz; ar(cellSz);
            construct(nx, ny, frames, cellSz);
        }
        // if you require versioning, simply add a const std::uint32_t as the final parameter, e.g.:
        // load_and_construct( Archive & ar, cereal::construct<MyType> & construct, std::uint32_t const version )
    };*/

    template <class Archive>
    struct specialize<Archive, TWMCResults, cereal::specialization::member_load_save> {};

    /*
    template <class Archive> inline
    std::string save( Archive  & ar, variables const & t )
    {
        ar(static_cast<int>(t));
    }

    template <class Archive> inline
    void load( Archive  & ar, variables & t )
    {
        int tmp;
        ar(tmp);
        t=static_cast<variables>(tmp);
    }*/

} // namespace ...

CEREAL_REGISTER_TYPE_WITH_NAME(TWMCResults, "TWMCResults")
CEREAL_REGISTER_POLYMORPHIC_RELATION(TaskResults, TWMCResults)
#endif



#endif /* TWMCResults_hpp */
