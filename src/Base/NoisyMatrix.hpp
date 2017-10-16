//
//  NoisyMatrix.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 04/10/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef NoisyMatrix_hpp
#define NoisyMatrix_hpp

#include <stdio.h>
#include "CustomTypes.h"
#include <random>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <tuple>

using namespace std;

class NoisyMatrix
{

public:
    NoisyMatrix(vector<size_t> dims);
    NoisyMatrix(size_t nx, size_t ny);

    void SetValue(complex_p val);
    void SetValue(float_p val);
    void SetValue(std::vector<float_p>& val);
    
    enum NoiseType { None, Uniform, Gaussian, Lorentzian, Poissonian};
    void SetNoiseType(string type);
    NoiseType GetNoiseType();

    size_t GetNoiseVariables();
    
    void SetNoiseVal(size_t id, float_p val);
    void SetNoiseVal(size_t id, complex_p val);
    void SetNoiseVal(size_t id, std::vector<float_p>& val);
    void SetTemporalValue(std::map<float_p, std::vector<float_p>> data);
    MatrixCXd GetAtTime(float_p t, size_t suggestedId = 0);
    tuple<size_t,MatrixCXd> GetAtTimeWithSuggestion(float_p t, size_t suggestedId = 0);

    MatrixCXd GenerateNoNoise();
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
    
    bool setup = false;
    bool noise = false;
    bool timeDep = false;
    
    std::vector<MatrixCXd> noiseVars;
    std::normal_distribution<> normal;
    std::uniform_real_distribution<> uniform;
    std::poisson_distribution<> poisson;
};

#endif /* NoisyMatrix_hpp */