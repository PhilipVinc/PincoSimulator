//
//  NoisyMatrix.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 04/10/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "NoisyMatrix.hpp"

#include "CustomTypes.h"
#include "Utils/EigenUtils.hpp"
#include "easylogging++.h"

#include <iostream>
#include <map>

/*// Fix for non standard isnan. Fixed in c++14
#if defined(__APPLE__)
#else
typedef std::isnan isnan;
#endif*/

using namespace std;

NoisyMatrix::NoisyMatrix(vector<size_t> dims)
    : normal(0.0, 1.0), uniform(-1.0, 1.0), poisson(1.0) {
  auto sz = dims.size();
  if (sz == 1) {
    nx = dims[0];
    ny = 1;  // dims[1];
  } else if (sz == 2) {
    nx = dims[0];
    ny = dims[1];
  } else if (sz == 3) {
    nx = dims[0];
    ny = dims[1] * dims[2];
  } else {
    cerr << "ERROR: matrix format not recognized." << endl;
  }
}

NoisyMatrix::NoisyMatrix(size_t _nx, size_t _ny, size_t cellSize)
    : normal(0.0, 1.0), uniform(0.0, 1.0), poisson(1.0) {
  nx = _nx;
  ny = _ny * cellSize;
}

void NoisyMatrix::SetValue(complex_p val) {
  baseMat = InitMatrix(nx, ny, val);
  setup   = true;
}

void NoisyMatrix::SetValue(float_p val) {
  baseMat = InitMatrix(nx, ny, val);
  setup   = true;
}

void NoisyMatrix::SetValue(const std::vector<float_p>& val) {
  baseMat = InitMatrix(nx, ny, val);
  setup   = true;
}

void NoisyMatrix::SetNoiseType(string type) {
  if (type == "Uniform" or type == "u") {
    noiseType = NoiseType::Uniform;
    noiseVars.resize(1);
  } else if (type == "Gaussian" or type == "g") {
    noiseType = NoiseType::Gaussian;
    noiseVars.resize(1);
  } else if (type == "Lorentzian" or type == "l") {
    noiseType = NoiseType::Lorentzian;
    noiseVars.resize(1);
  } else if (type == "Poissonian" or type == "p") {
    noiseType = NoiseType::Poissonian;
    noiseVars.resize(1);
  } else {
    noiseType = NoiseType::None;
    noiseVars.resize(0);
    if (type != "None") {
      LOG(ERROR) << "Unrecognized Noise Type " << type << ". Defaulting to: None.";
        LOG(ERROR) << "Possible types are None, Gaussian, Uniform, Lorentzian.";
    }
  }
}

size_t NoisyMatrix::GetNoiseVariables() const {
  switch (noiseType) {
    case Gaussian: return 1; break;

    case Uniform: return 1; break;

    case Lorentzian: return 1; break;

    case Poissonian: return 1; break;

    case None:

    default: return 0; break;
  }
}

void NoisyMatrix::SetNoiseVal(size_t id, float_p val) {
  noise         = true;
  noiseVars[id] = InitMatrix(nx, ny, val);
}

void NoisyMatrix::SetNoiseVal(size_t id, complex_p val) {
  noise         = true;
  noiseVars[id] = InitMatrix(nx, ny, val);
}

void NoisyMatrix::SetNoiseVal(size_t id, std::vector<float_p>& val) {
  noise         = true;
  noiseVars[id] = InitMatrix(nx, ny, val);
}

MatrixCXd NoisyMatrix::Generate(std::mt19937& gen) {
  if (setup) {
    if (!noise) {
      return baseMat;
    } else {
      return baseMat + EvaluateNoise(gen);
    }
  }

    LOG(ERROR) << "ERROR; Requested uninitialized matrix";
  return InitMatrix(nx, ny, 0.0);
}

MatrixCXd NoisyMatrix::EvaluateNoise(std::mt19937& gen) {
  MatrixCXd mat   = InitMatrix(nx, ny, 0.0 + ij * 0.0);
  size_t dim      = mat.rows() * mat.cols();
  complex_p* vals = mat.data();
  const complex_p* amplitudes;
  switch (noiseType) {
    case Gaussian:
      amplitudes = noiseVars[0].data();

      for (size_t i = 0; i < dim; i++) {
        vals[i] = normal(gen) * amplitudes[i].real() +
                  ij * normal(gen) * amplitudes[i].imag();
      }
      break;

    case Uniform:
      amplitudes = noiseVars[0].data();

      for (size_t i = 0; i < dim; i++) {
        vals[i] = uniform(gen) * amplitudes[i].real() +
                  ij * uniform(gen) * amplitudes[i].imag();
      }
      break;

      // case Lorentzian:
      // break;

    default:;
      // break;
  }
  return mat;
}

void NoisyMatrix::SetTemporalValue(
    std::map<float_p, std::vector<float_p>> data) {
  for (auto pt = data.begin(); pt != data.end(); pt++) {
    float_p timePt  = pt->first;
    MatrixCXd mat_t = MatrixCXd(nx, ny);
    complex_p* vals = mat_t.data();
    size_t dim      = nx * ny;

    if (pt->second.size() == nx * ny) {
      for (int i = 0; i != dim; ++i) { vals[i] = pt->second[i]; }
    } else if (pt->second.size() == nx * ny * 2) {
      for (int i = 0; i != nx * ny; i++) {
        vals[i] = pt->second[2 * i] + ij * pt->second[2 * i + 1];
      }

    } else {
        LOG(ERROR) << "Error in parsing a time-dependent matrix. Invalid number of "
              "elements."
           ;
    }

    times.push_back(timePt);
    cout << mat_t << endl;
    matCTimes.push_back(mat_t);
  }

  // Now I have to complete and fix the matrices
  for (int i = 0; i != nx; i++) {
    for (int j = 0; j != ny; j++) {
      for (int pt = 0; pt != times.size(); pt++) {
        complex_p matVal = matCTimes[pt](i, j);
        if (isnan(matVal.real()) || isnan(matVal.imag())) {
          complex_p previousVal = cNAN;
          int tprev;
          for (tprev = pt - 1; tprev >= 0 && isnan(previousVal.real());
               tprev--) {
            previousVal = matCTimes[tprev](i, j);
          }
          tprev++;
          complex_p nextVal = cNAN;
          int tnext;
          for (tnext = pt + 1; tnext < times.size() && isnan(nextVal.real());
               tnext++) {
            nextVal = matCTimes[tnext](i, j);
          }
          tnext--;

          // Check boundaries
          if (isnan(previousVal.real()) && isnan(nextVal.real())) {
              LOG(ERROR) << "ERROR: The Time-dependent matrix element (" << i << ", "
                 << j << ") has no known Elements!";
            break;
          } else if (isnan(nextVal.real()) && !isnan(previousVal.real())) {
            matCTimes[pt](i, j) = previousVal;
          } else if (!isnan(nextVal.real()) && isnan(previousVal.real())) {
            matCTimes[pt](i, j) = nextVal;
          } else {
            matCTimes[pt](i, j) =
                previousVal + (nextVal - previousVal) /
                                  (times[tnext] - times[tprev]) *
                                  (times[pt] - times[tprev]);
          }
        }
      }
    }
  }

  for (int i = 1; i != times.size(); i++) {
    cout << matCTimes[i - 1] << endl;
    dVals.push_back((matCTimes[i] - matCTimes[i - 1]) /
                    (times[i] - times[i - 1]));
  }
  dVals.push_back(matCTimes[0] - matCTimes[0]);
  dVals.push_back(matCTimes[0] - matCTimes[0]);
  matCTimes.push_back(matCTimes[times.size() - 1]);
  times.push_back(numeric_limits<float_p>::max());
  timeDep = true;

  for (int k = 0; k < times.size(); ++k) {
    LOG(INFO) << "t = " << times[k];
      LOG(INFO) << "F(t) = ";
      LOG(INFO) << matCTimes[k].real() ;
      LOG(INFO) << "dF(t) = " ;
      LOG(INFO) << dVals[k].real() ;
      LOG(INFO) << "------------------------";
  }
}

tuple<size_t, MatrixCXd> NoisyMatrix::GetAtTimeWithSuggestion(
    float_p t, size_t suggestedId) const {
  if (t <= times[0]) {
    return tuple<size_t, MatrixCXd>(0, matCTimes[0]);
  } else {
    size_t id;
    if (times[suggestedId + 1] > t)
      id = suggestedId;
    else
      id = lower_bound(times.begin(), times.end(), t) - times.begin() - 1;
    return tuple<size_t, MatrixCXd>(
        id, matCTimes[id] + complex_p(t - times[id], 0) * dVals[id]);
  }
}

bool NoisyMatrix::HasNoise() const {
  if (noiseType != NoiseType::None) return true;

  return false;
}
