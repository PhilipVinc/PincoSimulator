//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCSHAREDSYSTEMDATA_HPP
#define SIMULATOR_TWMCSHAREDSYSTEMDATA_HPP

#include "Base/TaskData.hpp"
#include "TWMCTypes.h"

#include <stdio.h>
#include <string>
#include <vector>

#ifdef MPI_SUPPORT
#include <boost/serialization/vector.hpp>
#include "../Libraries/eigen_boost_serialization.hpp"
#endif


class Settings;
class NoisyMatrix;


using namespace std;


class TWMCSystemData {
public:
	explicit TWMCSystemData(const Settings *settings);
    TWMCSystemData();
    ~TWMCSystemData();

    vector<float_p> GetStoredTimes() const;
    vector<vector<float_p>> GetStoredVariableEvolution(const NoisyMatrix* mat) const;

    std::string latticeName;
    size_t nx;
    size_t ny;
    size_t nxy;
    size_t cellSz;

    NoisyMatrix *U;
	NoisyMatrix *F;
	NoisyMatrix *omega;
    NoisyMatrix *E;
	MatrixCXd J;
	MatrixCXd gamma;

	MatrixCXd beta_init;

	float_p J_val;
    complex_p detuning;
    float_p J_AB_val, J_BC_val;
	float_p gamma_val;
	complex_p beta_init_val;
	float_p beta_init_sigma_val;
	float_p t_start;
	float_p t_end;
	float_p dt;
	float_p dt_obs;

	size_t n_dt;
	size_t n_frames;
	size_t frame_steps;


    bool PBC;
	enum Dimension {
		D0, D1, D2
	};
	Dimension dimension;

private:
#ifdef MPI_SUPPORT
	friend class boost::serialization::access;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & latticeName;
        ar & nx;
        ar & ny;
        ar & nxy;
        ar & cellSz;
        ar & F;
        ar & U;
        ar & omega;
        ar & E;
        ar & J;
        ar & gamma;
        ar & beta_init;
        ar & J_val;
        ar & detuning;
        ar & J_AB_val;
        ar & J_BC_val;
        ar & gamma_val;
        ar & beta_init_val;
        ar & beta_init_sigma_val;
        ar & t_start;
        ar & t_end;
        ar & dt;
        ar & dt_obs;
        ar & n_dt;
        ar & n_frames;
        ar & frame_steps;
        ar & PBC;
        ar & dimension;
    }
#endif

};

#endif //SIMULATOR_TWMCSHAREDSYSTEMDATA_HPP
