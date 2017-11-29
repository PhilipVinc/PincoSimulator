//
// Created by Filippo Vicentini on 27/11/2017.
//

#ifndef SIMULATOR_TWLIEBSIMDATA_HPP
#define SIMULATOR_TWLIEBSIMDATA_HPP


#include "TaskData.hpp"
#include "TWLiebTypes.hpp"

#include <stdio.h>
#include <string>
#include <vector>

class Settings;
class NoisyMatrix;


using namespace std;


class TWLiebSimData : public TaskData
{

public:

    TWLiebSimData(const Settings* settings);
    ~TWLiebSimData();

    NoisyMatrix* U_A;
    NoisyMatrix* U_B;
    NoisyMatrix* U_C;
    NoisyMatrix* F_A;
    NoisyMatrix* F_B;
    NoisyMatrix* F_C;
    NoisyMatrix* E_A;
    NoisyMatrix* E_B;
    NoisyMatrix* E_C;
    MatrixCXd J_AB;
    MatrixCXd J_BC;
    MatrixCXd gamma;

    MatrixCXd beta_init;

    float_p J_AB_val;
    complex_p detuning;
    float_p J_BC_val;
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

    size_t nx;
    size_t ny;
    size_t nxy;

    bool PBC;

    enum Dimension { D0, D1, D2 };
    Dimension dimension;

    vector<float_p> GetStoredTimes() const ;
    vector<vector<float_p>> GetStoredVariableEvolution(const NoisyMatrix* mat) const;
};

#endif //SIMULATOR_TWLIEBSIMDATA_HPP
