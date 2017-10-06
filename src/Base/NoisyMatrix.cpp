//
//  NoisyMatrix.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 04/10/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "NoisyMatrix.hpp"
#include "EigenUtils.hpp"
#include "CustomTypes.h"
#include <iostream>

NoisyMatrix::NoisyMatrix(vector<size_t> dims) : normal(0.0, 1.0), uniform(-1.0, 1.0), poisson(1.0)
{
    nx = dims[0];
    ny = dims[1];
}


NoisyMatrix::NoisyMatrix(size_t _nx, size_t _ny) : normal(0.0, 1.0), uniform(0.0, 1.0), poisson(1.0)
{
    nx = _nx;
    ny = _ny;
}


void NoisyMatrix::SetValue(complex_p val)
{
    baseMat=InitMatrix(nx, ny, val);
    setup=true;
}


void NoisyMatrix::SetValue(float_p val)
{
    baseMat=InitMatrix(nx, ny, val);
    setup=true;
}


void NoisyMatrix::SetValue(std::vector<float_p>& val)
{
    baseMat=InitMatrix(nx, ny, val);
    setup=true;
}

void NoisyMatrix::SetNoiseType(string type)
{
    if (type == "Uniform" or type == "u")
    {
        noiseType = NoiseType::Uniform;
        noiseVars.resize(1);
    }
    else if (type == "Gaussian" or type =="g")
    {
        noiseType = NoiseType::Gaussian;
        noiseVars.resize(1);
    }
    else if (type == "Lorentzian" or type =="l")
    {
        noiseType = NoiseType::Lorentzian;
        noiseVars.resize(1);
    }
    else if (type == "Poissonian" or type =="p")
    {
        noiseType = NoiseType::Poissonian;
        noiseVars.resize(1);
    }
    else
    {
        noiseType = NoiseType::None;
        noiseVars.resize(0);
        if (type != "None")
        {
            cerr << "Unrecognized Noise Type " << type <<". Defaulting to: None." << endl;
            cerr << "Possible types are None, Gaussian, Uniform, Lorentzian." << endl;
        }
    }
}

size_t NoisyMatrix::GetNoiseVariables()
{
    switch (noiseType) {
        case Gaussian:
            return 1;
            break;
            
        case Uniform:
            return 1;
            break;
            
        case Lorentzian:
            return 1;
            break;
            
        case Poissonian:
            return 1;
            break;

        case None:
            
        default:
            return 0;
            break;
    }
}

void NoisyMatrix::SetNoiseVal(size_t id, float_p val)
{
    noise = true;
    noiseVars[id] = InitMatrix(nx, ny, val);
}

void NoisyMatrix::SetNoiseVal(size_t id, complex_p val)
{
    noise = true;
    noiseVars[id] = InitMatrix(nx, ny, val);
}

void NoisyMatrix::SetNoiseVal(size_t id, std::vector<float_p>& val)
{
    noise = true;
    noiseVars[id] = InitMatrix(nx, ny, val);
}

MatrixCXd NoisyMatrix::GenerateNoNoise()
{
    return baseMat;
}

NoisyMatrix::NoiseType NoisyMatrix::GetNoiseType()
{
    return noiseType;
}

MatrixCXd NoisyMatrix::Generate(std::mt19937 &gen)
{
    if (setup)
    {
        if (!noise)
        {
            return baseMat;
        }
        else
        {
            return baseMat+EvaluateNoise(gen);
        }
    }
    
    cerr << "ERROR; Requested uninitialized matrix" << endl;
    return InitMatrix(nx, ny, 0.0);
}

MatrixCXd NoisyMatrix::EvaluateNoise(std::mt19937 &gen)
{
    MatrixCXd mat = InitMatrix(nx, ny, 0.0+ij*0.0);
    size_t dim = mat.rows()*mat.cols();
    complex_p* vals = mat.data();
    const complex_p* amplitudes;
    switch (noiseType)
    {
        case Gaussian:
            amplitudes = noiseVars[0].data();

            for (size_t i =0; i<dim ; i++)
            {
                vals[i] = normal(gen)*amplitudes[i].real() + ij*normal(gen)*amplitudes[i].imag();
            }
            break;
            
        case Uniform:
            amplitudes = noiseVars[0].data();
            
            for (size_t i =0; i<dim ; i++)
            {
                vals[i] = uniform(gen)*amplitudes[i].real() + ij*uniform(gen)*amplitudes[i].imag();
            }
            break;
            
       // case Lorentzian:
           // break;
            
            
        default:
            ;
           // break;
    }
    return mat;
}

