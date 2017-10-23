#  Installation

Prerequisites:

    - FFTW3 (on mac: brew install fftw )
    - Boost > 1.6.3
    - C++14 compiler 

To compile the code:
    
    cd projDir
    cmake -DCMAKE_BUILD_TYPE=Release .
    make sim

To run:

    ./sim -i INPUTFILE.ini (-o OUTPUTFOLDER)
    ./sim -i INPUTFODER (with INPUTFOLDER/_sim.ini existing)

check test-sim.ini for a sample input file

#  Noise Instructions

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
        * U_Noise_Type = NOISETYPE
        * U_Noise_Val_0 = NOISEVAR0
        * U_Noise_Val_1 = NOISEVAR1
        ...
        
        - NOISETYPES:
        *Gaussian
        *Uniform
        *None
        
    - The variables are either real/complex values or matrices, defined as specified before.
        - Noise_Val_0 = AMPLITUDE_Matrix
        


