#  File Format Pinco: Register

in Bytes

    0   ->  7       (8)     uchar[8]        Magic Bytes
    8   ->          (1)     uchar           Format Version Number
    9   ->  16      (8)     int             Number of Datasets
    
    repeated Number of Dataset times
        17 ->   24      (8)     int             Length of Dataset name (# of chars)
        25 ->   n       (8*n)   char[n]         DataSet Name

---- h

    For each Dataset (ex: heat, traj...)
    h   ->  h+1     (1)     size_t          Number of Dataset Dimensions @ each timestep
                                                ( 0= scalar, 1= 1D array, 2=2D matrix...)
	repeated Number of Dimensions times:
        dimension Length (1*N)  size_t          Length of each dimensions specified above
    
    
    For each Dataset (ex: heat, traj...)
        identifier of data type 
            11 --> float_p
            22 --> complex_p

---- K

    k     ->  k+7   (8)     uint          Size in bytes of extra data for each trajectory, stored in the Register (usually t0, tend)
    k+8   ->  k+15  (8)     uint          Size in bytes of extra global data stored in register (Z)
    k+16  ->  k+Z   (Z)     ****          Extra data
    
---- K+Z = O

Trajectory entries:

    T       ->  T+7     (8)     size_t   id
    T+8     ->  T+15    (8)     size_t   chunk_id
    T+16    ->  T+23    (8)     size_t   chunk_offset
    T+24    ->  T+31    (8)     size_t   continuation_offset
    T+32    ->  T+DD    (DD)    ******   additionalData



# File Format Chunk Register

Trajectory entries:
    T       ->  T+7     (8)     size_t   id

    fo each Dataset i :
    T + 8 + 16*d     ->  T + 8 + 16*d +7    (8)     size_t   elements in dataset
    T + 8 + 16*d + 8 ->  T + 8 + 16*d +15   (8)     size_t   offset in chunk


    