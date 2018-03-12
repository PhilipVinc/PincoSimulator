//
// Created by Filippo Vicentini on 06/02/2018.
//

#ifndef SIMULATOR_BOOSTSERIALIZATIONARCHIVEFORMATS_HPP
#define SIMULATOR_BOOSTSERIALIZATIONARCHIVEFORMATS_HPP

#ifdef MPI_SUPPORT
#include <cereal/archives/binary.hpp>
typedef cereal::BinaryInputArchive transmissionInputArchive;
typedef cereal::BinaryOutputArchive transmissionOutputArchive;

//#include <cereal/archives/json.hpp>
//typedef cereal::JSONInputArchive transmissionInputArchive;
//typedef cereal::JSONOutputArchive transmissionOutputArchive;
#endif

#endif //SIMULATOR_BOOSTSERIALIZATIONARCHIVEFORMATS_HPP
