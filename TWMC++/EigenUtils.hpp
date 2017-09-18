//
//  EigenUtils.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 08/12/16.
//  Copyright © 2016 Filippo Vicentini. All rights reserved.
//

#ifndef EigenUtils_hpp
#define EigenUtils_hpp

#include <stdio.h>
#include "TWMCTypes.h"

MatrixCXd InitDiagMatrix(size_t dim, size_t diag, complex_p value);

// Initialize with value in every site
MatrixCXd InitMatrix(size_t nx,size_t ny ,complex_p value);

// Initialize with value[i] at site i
MatrixCXd InitMatrix(size_t nx,size_t ny ,complex_p* value);
MatrixCXd InitMatrix(size_t nx,size_t ny ,float_p* value);

MatrixCXd MatExp(MatrixCXd mat);
MatrixCXd vecSqrt(MatrixCXd mat);
#endif /* EigenUtils_hpp */
