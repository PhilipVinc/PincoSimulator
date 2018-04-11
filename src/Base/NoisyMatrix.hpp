//
//  NoisyMatrix.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 04/10/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef NoisyMatrix_hpp
#define NoisyMatrix_hpp

#include "CustomTypes.h"

#include <stdio.h>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#ifdef MPI_SUPPORT
#include "Base/Serialization/SerializationArchiveFormats.hpp"

#include <cereal/access.hpp>
#include "../Libraries/eigen_cereal_serialization.hpp"
#endif

class NoisyMatrix {
 public:
  NoisyMatrix(std::vector<size_t> dims);
  NoisyMatrix(size_t nx, size_t ny, size_t cellSize = 1);

  void SetValue(complex_p val);
  void SetValue(float_p val);
  void SetValue(const std::vector<float_p> &val);

  enum NoiseType { None, Uniform, Gaussian, Lorentzian, Poissonian };
  void SetNoiseType(std::string type);
  inline NoiseType GetNoiseType() const { return noiseType; }
  bool HasNoise() const;
  inline bool HasTimeDependence() const { return timeDep; };
  inline size_t rows() {return nx;};
  inline size_t cols() {return ny;};

  size_t GetNoiseVariables() const;

  void SetNoiseVal(size_t id, float_p val);
  void SetNoiseVal(size_t id, complex_p val);
  void SetNoiseVal(size_t id, std::vector<float_p> &val);
  void SetTemporalValue(std::map<float_p, std::vector<float_p>> data);
  inline MatrixCXd GetAtTime(float_p t, size_t suggestedId = 0) const {
    return std::get<1>(GetAtTimeWithSuggestion(t, suggestedId));
  };
  std::tuple<size_t, MatrixCXd> GetAtTimeWithSuggestion(
      float_p t, size_t suggestedId = 0) const;
  inline std::tuple<size_t, size_t> GetDimensions() const { return {nx, ny}; };

  inline MatrixCXd GenerateNoNoise() { return baseMat; }
  MatrixCXd Generate(std::mt19937 &gen);
  MatrixCXd EvaluateNoise(std::mt19937 &gen);

 protected:
 private:
  std::vector<float_p> times;
  std::vector<MatrixCXd> dVals;
  std::vector<MatrixRXd> matRTimes;
  std::vector<MatrixCXd> matCTimes;

  std::vector<float_p> values;
  MatrixCXd baseMat;

  NoiseType noiseType = NoiseType::None;

  size_t nx;
  size_t ny;

  bool setup   = false;
  bool noise   = false;
  bool timeDep = false;

  std::vector<MatrixCXd> noiseVars;
  std::normal_distribution<> normal;
  std::uniform_real_distribution<> uniform;
  std::poisson_distribution<> poisson;

#ifdef MPI_SUPPORT
 public:
  friend class cereal::access;
  template <class Archive>
  void save(Archive &ar) const {
    ar(nx);
    ar(ny);

    ar(times);
    ar(dVals);
    ar(matRTimes);
    ar(matCTimes);
    ar(values);
    ar(baseMat);
    ar(noiseType);
    ar(setup);
    ar(noise);
    ar(timeDep);
    ar(noiseVars);
  }

  template <class Archive>
  void load(Archive &ar) {
    std::cout << "Loaded data for NoisyMatrix" << std::endl;

    ar(times);
    ar(dVals);
    ar(matRTimes);
    ar(matCTimes);
    ar(values);
    ar(baseMat);
    ar(noiseType);
    ar(setup);
    ar(noise);
    ar(timeDep);
    ar(noiseVars);
  }

  template <class Archive>
  static void load_and_construct(Archive &ar,
                                 cereal::construct<NoisyMatrix> &construct) {
    size_t nx, ny;
    ar(nx);
    ar(ny);

    construct(nx, ny);
    ar(construct->times);
    ar(construct->dVals);
    ar(construct->matRTimes);
    ar(construct->matCTimes);
    ar(construct->values);
    ar(construct->baseMat);
    ar(construct->noiseType);
    ar(construct->setup);
    ar(construct->noise);
    ar(construct->timeDep);
    ar(construct->noiseVars);
  }

#endif
};

#ifdef MPI_SUPPORT
namespace cereal {
template <class Archive>
struct specialize<Archive, NoisyMatrix,
                  cereal::specialization::member_load_save> {};
}  // namespace cereal
#endif

#endif /* NoisyMatrix_hpp */
