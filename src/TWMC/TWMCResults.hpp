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
#include "Base/Utils/VectorUtils.hpp"

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

// Deserialization methods
template <>
inline std::vector<complex_p> setData<std::vector<complex_p>>(rawTuple data, size_t frames, const std::vector<size_t>& dimensions) {
    size_t length = std::get<size_t>(data)/sizeof(complex_p);
    const complex_p * c_data = static_cast<const complex_p*>(std::get<const void*>(data));

    std::vector<complex_p> res(c_data, c_data+length);
    return res;
}

template <>
inline std::vector<float_p> setData<std::vector<float_p>>(rawTuple data, size_t frames, const std::vector<size_t>& dimensions) {
    size_t length = std::get<size_t>(data)/sizeof(float_p);
    const float_p * c_data = static_cast<const float_p*>(std::get<const void*>(data));

    std::vector<float_p> res(c_data, c_data+length);
    return res;
}

template <>
inline MatrixCXd setData<MatrixCXd>(rawTuple data, size_t frames, const std::vector<size_t>& dimensions) {
    size_t length = std::get<size_t>(data)/sizeof(complex_p);
    const complex_p * c_data = static_cast<const complex_p*>(std::get<const void*>(data));


    MatrixCXd res = Eigen::Map<const MatrixCXd>(c_data, dimensions[0], dimensions[1]);
    return res;
}

template <>
inline bool AppendData<std::vector<complex_p>>(std::vector<complex_p>& toExpand,
                                                    std::vector<complex_p>&& toAppend) {
  append(toAppend, toExpand); // src, dest
  return true;
}

template <>
inline bool AppendData<std::vector<float_p>>(std::vector<float_p>& toExpand,
                                                    std::vector<float_p>&& toAppend) {
  append(toAppend, toExpand); // src, dest
  return true;
}

template <>
inline bool AppendData<MatrixCXd>(MatrixCXd& toExpand, MatrixCXd&& toAppend) {
  toExpand = std::move(toAppend);
  return false;
}

typedef TaskResultsGeneric<TWMCData, std::array<double,2>, std::vector<complex_p>, std::vector<float_p>, MatrixCXd> TWMCResults;



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

static ResultsFactory::Registrator<TWMCResults> TWMCRes= ResultsFactory::Registrator<TWMCResults>("TWMC");

#endif /* TWMCResults_hpp */
