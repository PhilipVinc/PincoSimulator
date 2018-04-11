# PincoSimulator

A scalable Framework for parallel, distributed Montecarlo Trajectory-style physics simulation.

## Description

This framework is an ongoing work during my Ph.D. to help scale my numerical simulations. 
The idea is that if one specifies the common input and ouput of a numerical solver, the
framework will take care of the rest, namely multithreading, distributed processing, 
serialization during inter-process communication and Input/Ouput. Because the same problem 
might have, depending on the parameters, different 'best' solvers, it is easy to write
multiple solvers and select the best one at startup. Numerous points to plug oneself in
have been provided so that the system is heavily customizable.

The library also contains some experimental OpenCL kernels to run on the GPU, which 
are currently unstable.

This framework comes with a set of Matlab Utilities designed to read the standard binary 
data format, and perform some analysis if one desires. 


##  Installation

### Dependencies

Prerequisites:

    - C++14 compiler 
    - Boost >= 1.6 (program_options, system, filesystem)
    - FFTW3 
    - Eigen3 
    - CMake 3 (for generating the makefile)

Note: Boost::filesystem is not necessary if one has a C++17 compiler with Filesystem-TF 
support.

Optionals:

    - MPI Dispatch Support ( -DCOMPILE_MPI_SUPPORT=ON )
        - Cereal (>1.2)
        - MPI

    - GPU Kernels Support ( -DCOMPILE_GPU=ON )
        - OpenCL (>1.0)
        - VexCL

### Compilation

To compile the code you will need CMake 3 :
    
    cd PincoSimulator
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DCOMPILE_GPU=OFF ..
    make sim

To run:

    ./sim -i INPUTFILE.ini (-o OUTPUTFOLDER)
    ./sim -i INPUTFODER (with INPUTFOLDER/_sim.ini existing)

check test-sim.ini for a sample input file

## Usage Instructions

### Logging Options

    logpath : path to where a logfile should be stored
    nocout  : [true]/false - if we should print to std::cout what is logged to file (default false)

###  Noise Instructions

To define Specify a matrix U:

    - If the matrix is uniform (all values equal)
        - if it is real
            * U = value
        - if it is complex
            * U = re + i im
            or
            * U_real = re
            * U_imag = im

    - If the matrix has different values at each entry:
        * U = filename.dat
        and store into filename.dat the data for the matrix. If it is complex,
        interwine real and imaginary data in succession (so re(1,1) imag(1,1) re(1,2)...)
        and separate values with a tab.
    
To add noise to the matrix:

    - If there is NOISE set the following:
        * U.Noise_Type = NOISETYPE
        * U.Noise_Val_0 = NOISEVAR0
        * U.Noise_Val_1 = NOISEVAR1
        ...
        
        - NOISETYPES:
        *Gaussian
        *Uniform
        *None
        
    - The variables are either real/complex values or matrices, defined as specified before.
        - Noise_Val_0 = AMPLITUDE_Matrix
        


