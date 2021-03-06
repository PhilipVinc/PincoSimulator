//
//  CustomTypes.h
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef CustomTypes_h
#define CustomTypes_h

#include <complex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <Eigen/Dense>
#pragma clang pop

typedef float float_p32;
typedef float real_p32;
typedef std::complex<float_p32> complex_p32;
typedef Eigen::Matrix<complex_p32, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixCXd32;
typedef Eigen::Matrix<float_p32, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixRXd32;
typedef Eigen::Matrix<complex_p32, Eigen::Dynamic, 1> VectorCdCol32;
typedef Eigen::Matrix<complex_p32, 1, Eigen::Dynamic> VectorCdRow32;
const complex_p32 ij32(0.0,1.0);
const complex_p32 cNAN32(NAN, NAN);

typedef double float_p;
typedef float_p real_p;
typedef std::complex<float_p> complex_p;
typedef Eigen::Matrix<complex_p, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixCXd;
typedef Eigen::Matrix<float_p, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixRXd;
typedef Eigen::Matrix<complex_p, Eigen::Dynamic, 1> VectorCdCol;
typedef Eigen::Matrix<complex_p, 1, Eigen::Dynamic> VectorCdRow;
const complex_p ij(0.0,1.0);
const complex_p cNAN(NAN, NAN);



#endif /* CustomTypes_h */
