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

#include <map>
#include <random>
#include <stdio.h>
#include <string>
#include <tuple>
#include <vector>

#ifdef MPI_SUPPORT
#include "../Libraries/eigen_boost_serialization.hpp"
#endif

class NoisyMatrix
{

public:
    NoisyMatrix(std::vector<size_t> dims);
    NoisyMatrix(size_t nx, size_t ny, size_t cellSize = 1);

    void SetValue(complex_p val);
    void SetValue(float_p val);
    void SetValue(std::vector<float_p>& val);
    
    enum NoiseType { None, Uniform, Gaussian, Lorentzian, Poissonian};
    void SetNoiseType(std::string type);
    NoiseType GetNoiseType() const;
	bool HasNoise() const;
	bool HasTimeDependence() const;

    size_t GetNoiseVariables() const;
    
    void SetNoiseVal(size_t id, float_p val);
    void SetNoiseVal(size_t id, complex_p val);
    void SetNoiseVal(size_t id, std::vector<float_p>& val);
    void SetTemporalValue(std::map<float_p, std::vector<float_p>> data);
    MatrixCXd GetAtTime(float_p t, size_t suggestedId = 0) const;
	std::tuple<size_t,MatrixCXd> GetAtTimeWithSuggestion(float_p t, size_t suggestedId = 0) const;
	std::tuple<size_t, size_t> GetDimensions() const;

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

#ifdef MPI_SUPPORT
    friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & times;
		ar & dVals;
		ar & matRTimes;
		ar & matCTimes;
		ar & values;
		ar & baseMat;
		ar & noiseType;
		ar & nx;
		ar & ny;
		ar & setup;
		ar & noise;
		ar & timeDep;
		ar & noiseVars;
	}
#endif

};

#ifdef MPI_SUPPORT

// Support for serializing NoisyMatrix, as it has no default constructor
namespace boost {
	namespace serialization {
		template<class Archive>
		inline void save_construct_data(
				Archive & ar, const NoisyMatrix * t, const unsigned int file_version
		){
			// save data required to construct instance
			auto dims = t->GetDimensions();
			ar & std::get<0>(dims);
			ar & std::get<1>(dims);
		}

		template<class Archive>
		inline void load_construct_data(
				Archive & ar, NoisyMatrix * t, const unsigned int file_version
		){
			// retrieve data from archive required to construct new instance
			size_t nx;	ar & nx;
			size_t ny;  ar & ny;

			// invoke inplace constructor to initialize instance of my_class
			::new(t)NoisyMatrix(nx, ny);
		}
	}} // namespace ...
#endif


#endif /* NoisyMatrix_hpp */
