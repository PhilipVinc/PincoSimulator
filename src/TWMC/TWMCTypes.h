//
//  TWMCTypes.h
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCTypes_h
#define TWMCTypes_h

#include "CustomTypes.h"

#include <fftw3.h>


struct TWMC_FFTW_plans
{
    MatrixCXd fft_f_in;
    MatrixCXd fft_f_out;
    MatrixCXd fft_i_in;
    MatrixCXd fft_i_out;
    
    fftw_plan forward_fft;
    fftw_plan inverse_fft;
};

#endif /* TWMCTypes_h */
