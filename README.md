# 🔴 PincoSimulatorFramework (and TWMCSimulator) 🔴

A scalable Framework for parallel, distributed Montecarlo Trajectory-style physics simulation.
Several TWMC simulation kernels is included.

## Description

### Framework

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

### TWMC

A set of Truncated Wigner Monte Carlo (TWMC) kernels are included with the code. It is possible
to simulate 1D, 2D or 1D Lieb Driven-Dissipative Bose-Hubbard lattices with or without PBC 
and with or without disorder. In the case of uniform hopping constant, a Fast Fourier Transform
is used to compute the dispersion contribution in k-space, with a cost O(Nlog(N)) instead of 
O(N^2).


##  Installation

### Dependencies

Prerequisites:
	
	Hard Dependencies
    - C++14 compiler 
    - CMake 3 (for generating the makefile)
    - Boost::filesystem >= 1.6 (or C++17Filesystem)
    - FFTW3 
    
    Header only libraries (included)
    - Eigen3 
    - CLI 11 (Input parsing)
    - Easylogging++ (logging library)
    - CPPTOML (.toml/.ini file parsing)

Note: Boost::filesystem is not necessary if one has a C++17 compiler with Filesystem-TF 
support.

Optionals:

    - MPI Dispatch Support ( -DCOMPILE_MPI=ON )
        - Cereal (>1.2)
        - MPI

    - GPU Kernels Support ( -DCOMPILE_GPU=ON )
        - OpenCL (>1.0)
        - VexCL

### Compilation

To compile the code you will need CMake 3:
    
    cd PincoSimulator
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DCOMPILE_GPU=OFF ..
    make -j 8 sim

#### Cmake build Options

    COMPILE_GPU [Default=OFF] Enable GPU Support for supported solvers.
    COMPILE_MPI [Default=OFF] Enable MPI Support for all Managers.
    PROFILE_MPI [Default=OFF] Enable MPE Profiling Layer for MPI.

    DEIGEN3_INCLUDES Path to the parent folder of the Eigen3/ header folder.
    DCEREAL_INCLUDES Path to the parent folder of the Cereal/ header folder.

## Usage Instructions

To run:

    ./sim -i INPUTFILE.ini (-o OUTPUTFOLDER)
    ./sim -i INPUTFODER (with INPUTFOLDER/_sim.ini existing)

check test-sim.ini for a sample input file

### Options

#### Logging

    logpath     : path to where a logfile should be stored
    nocout      : [true]/false - if we should print to std::cout what is logged to file (default false)

#### General 

    chunk_size  : [default=4] maximum chunk size (in GB)
    ppn         : [default=softhreads] maximum number of threads per process.

###  Matrix Input Instructions

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
        


