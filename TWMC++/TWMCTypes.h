//
//  TWMCTypes.h
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCTypes_h
#define TWMCTypes_h

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
const complex_p ij(0.0,1.0);


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
    unsigned int seed;
};

#endif /* TWMCTypes_h */
