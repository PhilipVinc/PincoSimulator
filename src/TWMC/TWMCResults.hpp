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

#include <stdio.h>

class TWMCTaskData;

#ifdef MPI_SUPPORT
#include "../Base/MPIManager/BoostSerializationArchiveFormats.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#endif

class TWMCResults : public TaskResults
{
public:
    TWMCResults(const TWMCTaskData* TaskData);
    TWMCResults(size_t nx, size_t ny, size_t frames, size_t _cellSz);
    ~TWMCResults();

    std::vector<complex_p> beta_t;
    std::vector<float_p> work_t;
    std::vector<float_p> area_t;
    //std::vector<complex_p*> complexMatrices;
    std::vector<std::vector<complex_p>*> complexMatrices;
    //std::vector<float_p*> realMatrices;
    std::vector<std::vector<float_p>*> realMatrices;
    size_t nx;
    size_t ny;
    size_t cellSz;
    size_t nxy;
    size_t frames;
    
    double extraDataMemory[2];
    void AddRealMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames,  size_t cellSz);
    void AddComplexMatrixDataset(std::string name, size_t nx, size_t ny, size_t frames,
                                     size_t cellSz);

    // Serialization and deserialization
    const virtual unsigned int SerializingExtraDataOffset()const;
    const virtual void* SerializeExtraData()const;
    virtual void DeSerializegExtraData(void* data, unsigned int length);


protected:


private:
#ifdef MPI_SUPPORT
    friend class boost::serialization::access;
/*     template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {

        ar & beta_t;
        ar & complexMatrices;
        ar & realMatrices;

        ar & work_t;
        ar & area_t;

        ar & frames;
        ar & extraDataMemory[0];
        ar & extraDataMemory[1];

        ar & boost::serialization::base_object<TaskResults>(*this);
    };
*/
    /*template<class Archive> void save(Archive & ar, const unsigned int version)
     {
         ar & beta_t;
         ar & complexMatrices;
         ar & realMatrices;

         ar & work_t;
         ar & area_t;

         ar & nx;
         ar & ny;
         ar & cellSz;
         ar & nxy;
         ar & frames;
         ar & extraDataMemory[0];
         ar & extraDataMemory[1];

         ar & boost::serialization::base_object<TaskResults>(*this);
     }

     template<class Archive> void load(Archive & ar, const unsigned int version)
     {
         ar & beta_t;
         ar & complexMatrices;
         ar & realMatrices;

         ar & work_t;
         ar & area_t;

         ar & nx;
         ar & ny;
         ar & cellSz;
         ar & nxy;
         ar & frames;
         ar & extraDataMemory[0];
         ar & extraDataMemory[1];

         ar & boost::serialization::base_object<TaskResults>(*this);
     }*/
#endif


};

#ifdef MPI_SUPPORT
//BOOST_CLASS_EXPORT_KEY2(TWMCResults, "TWMCResults")
#endif

#ifdef MPI_SUPPORT

// Support for serializing NoisyMatrix, as it has no default constructor
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(
                Archive & ar, const TWMCResults * t, const unsigned int file_version
        ){
            // save data required to construct instance
            ar & t->nx;
            ar & t->ny;
            ar & t->frames;
            ar & t->cellSz;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive & ar, TWMCResults * t, const unsigned int file_version
        ){
            // retrieve data from archive required to construct new instance
            size_t nx;	ar & nx;
            size_t ny;  ar & ny;
            size_t frames; ar & frames;
            size_t cellSz; ar & cellSz;

            // invoke inplace constructor to initialize instance of my_class
            ::new(t)TWMCResults(nx, ny, frames, cellSz);
        }
    }} // namespace ...
#endif




#endif /* TWMCResults_hpp */
