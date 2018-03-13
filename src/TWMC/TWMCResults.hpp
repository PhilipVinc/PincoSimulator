//
//  TWMCResults.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCResults_hpp
#define TWMCResults_hpp

#include "Base/TaskResultsGeneric.hpp"
#include "TWMCTypes.h"

#include <array>
#include <stdio.h>
#include <map>
#include <tuple>

class TWMCTaskData;

#ifdef MPI_SUPPORT
#include <cereal/cereal.hpp>
#endif

enum TWMCData { traj, work, area,
    U_Noise, F_Noise, Delta_Noise, J_Noise};



template<>
inline rawTuple getData<std::vector<complex_p>>(const std::vector<complex_p>& data) {
    return std::make_tuple<rawT1, rawT2>(data.data(), data.size()*sizeof(complex_p));
};
template<>
inline rawTuple getData<std::vector<float_p>>(const std::vector<float_p>& data) {
    return std::make_tuple<rawT1, rawT2>(data.data(), data.size()*sizeof(float_p));
};
template<>
inline rawTuple getData<MatrixCXd>(const MatrixCXd& data) {
    return std::make_tuple<rawT1, rawT2>(data.data(), data.size()*sizeof(complex_p));
};


typedef TaskResultsGeneric<TWMCData, double[2], std::vector<complex_p>, std::vector<float_p>, MatrixCXd> TWMCResults;


#ifdef MPI_SUPPORT

// Declare how to serialize this class. Must be like this
namespace cereal {

    template <class Archive>
    struct specialize<Archive, TWMCResults, cereal::specialization::member_serialize> {};

} // namespace ...

CEREAL_REGISTER_TYPE_WITH_NAME(TWMCResults, "TWMCResults")
CEREAL_REGISTER_POLYMORPHIC_RELATION(TaskResults, TWMCResults)
#endif

namespace dummy2 {
    struct bogus2 {
        static int bogus_method();
    };
}

// force linking with util.cc
static int bogus_variable = dummy2::bogus2::bogus_method();


#endif /* TWMCResults_hpp */
