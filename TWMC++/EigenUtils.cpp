//
//  EigenUtils.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 08/12/16.
//  Copyright © 2016 Filippo Vicentini. All rights reserved.
//

#include "EigenUtils.hpp"
const complex_p ij(0.0,1.0);

MatrixCXd InitDiagMatrix(size_t dim, size_t diag, complex_p value)
{
    MatrixCXd mat = MatrixCXd::Zero(dim, dim);
    for (size_t i = 0; i < dim; ++i)
    {
        for (size_t j = 0; j < dim; ++j)
        {
            if (i==j-diag)
            {
                mat(i,j) = value;
            }
            else
            {
                mat(i,j) = 0.0;
            }
        }
    }
    return mat;
}

MatrixCXd InitMatrix(size_t nx,size_t ny ,complex_p value)
{
    size_t nxy = nx*ny;
    MatrixCXd vec = MatrixCXd(nx,ny);
    for (size_t i=0; i < nxy; i++)
    {
        vec(i) = value;
    }
    return vec;
}

MatrixCXd InitMatrix(size_t nx,size_t ny ,complex_p* value)
{
    size_t nxy = nx*ny;
    MatrixCXd mat = MatrixCXd(nx,ny);
    if (nx==1 || ny==1)
    {
        for (size_t i=0; i < nxy; i++)
        {
            mat(i) = value[i];
        }
    }
    else
    {
        for (size_t i = 0, nRows = mat.rows(), nCols = mat.cols(); i < nRows; ++i)
        {
            for (size_t j = 0; j < nCols; ++j)
            {
                mat(i,j) = value[j*nRows+i];
            }
        }
    }
    return mat;
}

MatrixCXd InitMatrix(size_t nx,size_t ny ,float_p* value)
{
    size_t nxy = nx*ny;
    MatrixCXd mat = MatrixCXd(nx,ny);
    if (nx==1 || ny==1)
    {
        for (size_t i=0; i < nxy; i++)
        {
            mat(i) = value[i];
        }
    }
    else
    {
        for (size_t i = 0, nRows = mat.rows(), nCols = mat.cols(); i < nRows; ++i)
        {
            for (size_t j = 0; j < nCols; ++j)
            {
                mat(i,j) = value[j*nRows+i];
            }
        }
    }
    return mat;
}


MatrixCXd MatExp(MatrixCXd mat)
{
    MatrixCXd expmat = MatrixCXd(mat.rows()*mat.cols(), 1);
    for (size_t i = 0, nRows = mat.rows(), nCols = mat.cols(); i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            expmat(i,j) = exp(mat(i,j));
        }
    }
    return expmat;
}

void DebugCheck(TWMC_Results* res, float time)
{
    // Assert(check_boundaries(res))
    // Pause for syncing threads
    
}

MatrixCXd vecSqrt(MatrixCXd mat)
{
    MatrixCXd newmat = MatrixCXd(mat.rows()*mat.cols(), 1);
    for (size_t i = 0, nRows = mat.rows(), nCols = mat.cols(); i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            newmat(i,j) = sqrt(mat(i,j));
        }
    }
    return newmat;
}




