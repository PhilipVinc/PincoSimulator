//
//  TWMC_structures.h
//  TWMC-test
//
//  Created by Filippo Vicentini on 19/11/16.
//  Copyright © 2016 Filippo Vicentini. All rights reserved.
//

#ifndef TWMC_structures_h
#define TWMC_structures_h

#include <complex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <Eigen/Dense>
#pragma clang pop

#include <fftw3.h>

typedef double float_p;
typedef std::complex<float_p> complex_p;
typedef Eigen::Matrix<complex_p, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixCXd;
typedef Eigen::Matrix<complex_p, Eigen::Dynamic, 1> VectorCdCol;
typedef Eigen::Matrix<complex_p, 1, Eigen::Dynamic> VectorCdRow;

struct TWMC_Data
{
    MatrixCXd U;
    MatrixCXd J;
    MatrixCXd F;
    MatrixCXd gamma;
    MatrixCXd omega;
    MatrixCXd beta_init;
    
    float_p U_val;
    float_p J_val;
    float_p F_val;
    float_p gamma_val;
    float_p omega_val;
    complex_p beta_init_val;
    float_p beta_init_sigma_val;
    float_p t_start;
    float_p t_end;
    float_p dt;
    float_p dt_obs;
    
    size_t n_dt;
    size_t n_frames;
    
    size_t nx;
    size_t ny;
    size_t nxy;
    
};

struct TWMC_FFTW_plans
{
    MatrixCXd fft_f_in;
    MatrixCXd fft_f_out;
    MatrixCXd fft_i_in;
    MatrixCXd fft_i_out;
    
    fftw_plan forward_fft;
    fftw_plan inverse_fft;
};

struct TWMC_Results
{
    complex_p* beta_t;
    size_t id;
    size_t n;
    size_t nx;
    size_t ny;
};

#endif /* TWMC_structures_h */
