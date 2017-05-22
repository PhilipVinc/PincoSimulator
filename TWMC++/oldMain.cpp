//
//  main.cpp
//  TWMC-test
//
//  Created by Filippo Vicentini on 19/11/16.
//  Copyright © 2016 Filippo Vicentini. All rights reserved.
//

//  DATA IS SAVED AS:
//  X1Y1    X1Y2    X1Y3    ... X2Y1    X2Y2    X2Y3...
//

#include <iostream>
#include <complex>
#include <cmath>
#include <thread>
#include <vector>
#include <random>
#include <queue>
#include <tuple>
#include <time.h>
#include <stdio.h>
#include <chrono>
#include <fstream>
#include "IniPrefReader.hpp"
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <iomanip>



#include "TWMC_structures.h"
#include "fftw3.h"
#include "TWMC_evolve.hpp"
#include "WorkerThread.hpp"
#include "EigenUtils.hpp"

void usage(const char * pname);
uint file_exist(const std::string fileName);
void CheckCreateFolder(std::string folderName);
void CopyFileToFolder(std::string inputFile, std::string outputFolder);
std::vector<std::string> findAllFolders(std::string path);
std::vector<std::string> findFoldersContainingString(std::string path, std::string pattern);

using namespace std;

float_p* CreateRealVector(int dim, float_p val = 0);
complex_p* CreateComplexVector(int dim, complex_p val = 0);
complex_p* RealToComplexArray(int dim, float_p* arr);
void SumVectorOnFirst(complex_p* v1, complex_p* v2, int dim);
void SumVectorOnFirst(float_p* v1, float_p* v2, int dim);
void PrintTimesFile(TWMC_Data& dat, string outputPath);

TWMC_Data* SetupData(IniPrefReader* prefs);

const string paramsFileName = "_sim.ini";
const string folderSeparator = "/";
const string trajectoryFolderBaseName = "trajectories";
const string trajectoryNames = "traj";
const string trajectoryExtension = ".bin";

int main(int argc, char * argv[])
{
    // Command Line options flags and values
    string inputPath = "rien";
    string outputPath = "lastSim";
    bool continuationOfOldSim = false;
    bool outputSet = false;
    unsigned long trajectoryFoldersN = 1;
    // Parse input file name from commandline args
    int opt;
    while((opt = getopt(argc, argv, "i:o:h?")) != -1)
    {
        switch (opt)
        {
            case 'i':
                inputPath = optarg;
                break;
            case 'o':
                outputPath = optarg;
                outputSet = true;
                break;
            case 'h':
            case '?':
            default:
                usage( argv[0] );
                return 1;
        }
    }
    uint fE = file_exist(inputPath);
    if (fE == 2)
    {
        cout << "Loading Folder: " << inputPath << endl;
        outputPath = inputPath;
        inputPath = inputPath + folderSeparator + paramsFileName;
        fE = file_exist(inputPath);
        cout << "New params file is: " << inputPath << endl;
        continuationOfOldSim = true;
        
        auto tmppp = findFoldersContainingString(outputPath, trajectoryFolderBaseName);
        auto tmpN = findFoldersContainingString(outputPath, trajectoryFolderBaseName).size();
        for (int i =0; i < tmpN; i++)
            cout <<tmppp[i] << endl;
        trajectoryFoldersN = findFoldersContainingString(outputPath, trajectoryFolderBaseName).size()+1;
    }
    if (fE == 0)
    {
        std::cerr << "Did not provide input data. Shutting down."<< std::endl;
        usage( argv[0] );
        return 0;
    }
    
    //
    
    // Parse the input file.
    IniPrefReader* prefs = new IniPrefReader(inputPath);
    string simName = prefs->getOptionToString("save_path");
    if (simName != "" &&  !continuationOfOldSim && !outputSet)
        outputPath = simName;
    outputPath += "/";
    
    cout << "Reading Ini File: " << inputPath << endl;
    cout << "Saving to Folder: " << outputPath << endl;
    
    CheckCreateFolder(outputPath);
    
    if (!continuationOfOldSim)
    {
        CopyFileToFolder(inputPath, outputPath);
    }
    
    
    // Load all the data from the input file. VERY IMPORTANT METHOD
    TWMC_Data* simdata = SetupData(prefs);
    size_t nx = simdata->nx;
    size_t ny = simdata->ny;
    
    int n_traj = prefs->getOptionToInt("n_traj");
    
    // Compute the timestep
    int timestep_factor = prefs->getOptionToInt("timestep_factor");
    float_p vals_array[] = {abs(simdata->U_val), abs(simdata->J_val), abs(simdata->F_val), abs(simdata->omega_val)};
    float_p w_max = *max_element(vals_array,vals_array+4);
    int n_times = round(log2((simdata->t_end - simdata->t_start)*w_max));
    simdata->dt = (simdata->t_end - simdata->t_start)/pow(2,n_times+timestep_factor);
    simdata->dt_obs = simdata->n_frames == 0 ? 0.0 :(simdata->t_end - simdata->t_start)/simdata->n_frames;
    simdata->n_dt = (simdata->t_end - simdata->t_start)/simdata->dt;
    
    // Fix the dt_timestep
    if (simdata->dt_obs < simdata->dt)
    {
        simdata->dt_obs = simdata->dt;
        simdata->n_frames = simdata->n_dt;
    }
    
    // Select 1D or 2D FFT
    bool is1D = ((simdata->ny == 1) || (simdata->nx == 1)) ? true : false;
    is1D ? cout << "1D FFT Selected" << endl : cout << "2D FFT Selected" << endl;
    
    // Single output file?
    bool singleOutputFile = prefs->getOptionToInt("singleOutputFile");
    
    // Initialize the RNG generator generating the seeds for each simulations
    unsigned int SEED = prefs->getOptionToInt("SEED");
    if (SEED == 0)
        SEED = (unsigned int)time(0);
    else if (continuationOfOldSim)
        SEED = SEED + (unsigned int)time(0);
    std::mt19937 seedGenerator(SEED);
    
    size_t num_threads = prefs->getOptionToInt("processes");
    if(num_threads > n_traj)
        num_threads = n_traj;
    
    // Instantiate the array holding the threads
    cout << "singleOutputFile = " << singleOutputFile << endl;
    cout << "Global Seed = " << SEED << endl;
    cout << "U= " << prefs->getOptionToFloat("U") << endl;
    cout << "Delta= " << prefs->getOptionToFloat("omega") << endl;
    cout << "J= " << prefs->getOptionToFloat("J") << endl;
    cout << "F= " << prefs->getOptionToFloat("F") << endl;
    cout << "beta_init= " << simdata->beta_init_val << endl;
    cout << "beta_init_var= " << simdata->beta_init_sigma_val << endl;
    cout << "num_traj= " << prefs->getOptionToInt("n_traj") << endl;
    cout << "dt= " << simdata->dt << endl;
    cout << "n_timesteps= " << simdata->n_dt << " divided by dt_obs = " << simdata->dt_obs << endl;
    cout << "will save N_Frames = " << simdata->n_frames << endl;
    cout << "data save format = X1Y1    X1Y2    X1Y3    ... X2Y1    X2Y2    X2Y3..." <<endl;
    cout << "----------------------------------" << endl;
    cout << "      Running with " << num_threads << " threads" << endl;
    cout << "----------------------------------" << endl;
    
    prefs->ClosePrefFile();
    
    // Print Times
    PrintTimesFile(*simdata, outputPath);
    
    // Parallel stuff initialization: create vectors to hold workers,
    // results and the queue for filesystem write of elaborated data.
    std::thread * threads = new thread[num_threads];
    std::vector<WorkerThread*> workers(num_threads);
    std::vector<TWMC_FFTW_plans*> fftw_plans(num_threads);
    std::vector<TWMC_Results*> results(num_threads);
    std::queue<TWMC_Results*> elaboratedResults;
    unsigned int tmpSeed = 0;
    
    // Time handling
    int sleepTimeMs = 10;
    std::vector<time_t> startTimes(num_threads);
    auto t_start = std::chrono::steady_clock::now();
    auto lastPrintTime = std::chrono::duration<double, std::milli>(t_start-t_start).count();
    float averageComputationTime = 1000;
    int computedTraj = 0;
    
    // File names preparation
    const string trajectoryFolder = outputPath + folderSeparator + trajectoryFolderBaseName + to_string(trajectoryFoldersN) + folderSeparator;
    const string trajectoryPrepend = trajectoryFolder + trajectoryNames;
    CheckCreateFolder(trajectoryFolder);
    cout << "will save trajectories to Folder: " << trajectoryFolder << endl;
    
    FILE* ofile = nullptr;
    if (singleOutputFile)
    {
        ofile = fopen((trajectoryPrepend+"All"+trajectoryExtension).c_str(),"wb");
        //ofile.open(outputPath+"/computedTrajAll"+".dat", std::ofstream::out);// | std::ofstream::app );
    }
    cout << std::setprecision(2)<< std::fixed;
    
    // Id holding computation #
    int calcId = 0;
    
    // Instantiate the peons! (workers) and start the first round.
    // Should abstract to external class.
    for (int i = 0; i<num_threads; i++)
    {
        workers[i] = new WorkerThread(i);
        threads[i] = std::thread(&WorkerThread::WorkerLoop, workers[i]);
        fftw_plans[i] = new TWMC_FFTW_plans;
        
        // Code to initialize the reusable fftw plans (necessary for multithreading. uff)
        fftw_plans[i]->fft_f_in = MatrixCXd::Zero(nx, ny);
        fftw_plans[i]->fft_f_out = MatrixCXd::Zero(nx, ny);
        fftw_plans[i]->fft_i_in = MatrixCXd::Zero(nx, ny);
        fftw_plans[i]->fft_i_out = MatrixCXd::Zero(nx, ny);
        
        complex_p* f_in_ptr = fftw_plans[i]->fft_f_in.data();
        complex_p* f_out_ptr = fftw_plans[i]->fft_f_out.data();
        complex_p* i_in_ptr = fftw_plans[i]->fft_i_in.data();
        complex_p* i_out_ptr = fftw_plans[i]->fft_i_out.data();
        
        
        if (is1D)
        {
            fftw_plans[i]->forward_fft = fftw_plan_dft_1d(int(nx*ny),
                                                          reinterpret_cast<fftw_complex*>(f_in_ptr),
                                                          reinterpret_cast<fftw_complex*>(f_out_ptr),
                                                          FFTW_FORWARD,
                                                          FFTW_MEASURE);
            fftw_plans[i]->inverse_fft = fftw_plan_dft_1d(int(nx*ny),
                                                          reinterpret_cast<fftw_complex*>(i_in_ptr),
                                                          reinterpret_cast<fftw_complex*>(i_out_ptr),
                                                          FFTW_BACKWARD,
                                                          FFTW_MEASURE);
        }
        else
        {
            fftw_plans[i]->forward_fft = fftw_plan_dft_2d(int(nx), int(ny),
                                                          reinterpret_cast<fftw_complex*>(f_in_ptr),
                                                          reinterpret_cast<fftw_complex*>(f_out_ptr),
                                                          FFTW_FORWARD,
                                                          FFTW_MEASURE);
            fftw_plans[i]->inverse_fft = fftw_plan_dft_2d(int(nx), int(ny),
                                                          reinterpret_cast<fftw_complex*>(i_in_ptr),
                                                          reinterpret_cast<fftw_complex*>(i_out_ptr),
                                                          FFTW_BACKWARD,
                                                          FFTW_MEASURE);
        }
        
        TWMC_Results *res = new TWMC_Results;
        tmpSeed = seedGenerator();
        
        res->n = simdata->n_frames;
        res->nx = simdata->nx;
        res->ny = simdata->ny;
        res->id = calcId;
        
        res->seed = tmpSeed;
        
        calcId++;
        res->beta_t = new complex_p[simdata->n_frames*simdata->nxy];
        
        results[i] = res;
        workers[i]->AssignPlan(fftw_plans[i]);
        workers[i]->AssignSimulatorData(simdata, res, tmpSeed);
        startTimes[i] = time(NULL);
    }
    
    // Main loop executed ad libitum (until all the necessary simulations
    // have been run.
    while (computedTraj < n_traj)
    {
        // Iterate through all the workers and check if they are done.
        // and if so, feed them a new simulation and store their result.
        for (int i = 0; i != num_threads; i++)
        {
            if (workers[i]->IsFinished())
            {
                elaboratedResults.push(results[i]);
                workers[i]->ClearSimulator();
                computedTraj++;
                
                if (calcId < n_traj)
                {
                    TWMC_Results *res = new TWMC_Results;
                    res->n = simdata->n_frames;
                    res->nx = simdata->nx;
                    res->ny = simdata->ny;
                    res->id = calcId; calcId++;
                    res->beta_t = new complex_p[simdata->n_frames*simdata->nxy];
                    
                    tmpSeed = seedGenerator();
                    res->seed = tmpSeed;
                    
                    results[i] = res;
                    workers[i]->AssignSimulatorData(simdata, res, tmpSeed);
                    startTimes[i] = time(NULL);
                }
                else
                {
                    workers[i]->AssignSimulatorData(nullptr,
                                                    nullptr,
                                                    (unsigned int)NULL);
                }
            }
        }
        
        auto t_now = std::chrono::steady_clock::now();
        double elapsedTime = std::chrono::duration<double, std::milli>(t_now-t_start).count();
        
        if (!elaboratedResults.empty() && computedTraj > 0)
        {
            averageComputationTime = elapsedTime / double(computedTraj)*double(n_traj);
            sleepTimeMs = elapsedTime / double(computedTraj) * double(num_threads)/10;
            if (sleepTimeMs > 1000)
                sleepTimeMs = 1000;
        }
        
        // Write to the filesystem elaborated data in the queue.
        while (!elaboratedResults.empty())
        {
            TWMC_Results res = *elaboratedResults.front();
            
            size_t size = res.nx*res.ny;
            
            if (!singleOutputFile)
            {
                ofile = fopen((trajectoryPrepend+to_string(res.seed)+trajectoryExtension).c_str(), "wb");
            }
            fwrite(res.beta_t, 1, sizeof(complex_p)*res.n*size, ofile);
            
            if (!singleOutputFile)
            {
                fclose(ofile);
            }
            delete[] res.beta_t;
            delete elaboratedResults.front();
            elaboratedResults.pop();
        }
        
        // Print the progress
        
        double timeIncrement = averageComputationTime/100.0;
        if ((lastPrintTime + 1000 < elapsedTime) && ((elapsedTime - lastPrintTime) / timeIncrement >= 1) && computedTraj!= 0)
        {
            lastPrintTime = elapsedTime;
            cout << "Computed " << computedTraj << " / " << n_traj << " trajectories. Time: " << int(elapsedTime/1000) << " / " << int(averageComputationTime/1000) << "   (Sleep="<<double(sleepTimeMs)/1000<<" s)"<<endl;
            
        }
        // It's useless for this control while loop to be run many times, a couple of times per second
        // is more than enough. If a task lasts for 10 seconds it will never wait for than 0.5 seconds to be
        // fed a new simulation, and we are freeing up one core for additional computations, which guarantee
        // 9.5s more of computation time.
        std::this_thread::sleep_for (std::chrono::milliseconds(sleepTimeMs));
    }
    
    if (singleOutputFile)
        fclose(ofile);
    
    // We're done. Kill the peoons.
    for (int i = 0; i<num_threads; i++)
    {
        workers[i]->Terminate();
        threads[i].join();
    }
    
    cout << "Simulation completed" << endl;
    return 0;
}

TWMC_Data* SetupData(IniPrefReader* prefs)
{
    TWMC_Data* simData = new TWMC_Data;
    
    size_t nx = prefs->getOptionToInt("nx");
    size_t ny = prefs->getOptionToInt("ny");
    
    if(ny == 1)
    {
        cout << "1D System with " << nx << " x 1" << " sites."<< endl;
        cout <<"Reshaping to 1 x " << nx << " for calculation. Output will be the same." << endl;
        ny = nx;
        nx = 1;
    }
    
    simData->nx = nx;
    simData->ny = ny;
    simData->nxy = nx*ny;
    
    
    // Read the predata
    simData->U_val = prefs->getOptionToFloat("U");
    simData->J_val = prefs->getOptionToFloat("J");
    simData->F_val = prefs->getOptionToFloat("F");
    simData->omega_val = prefs->getOptionToFloat("omega");
    simData->gamma_val = prefs->getOptionToFloat("gamma");
    simData->beta_init_val = complex_p(1,0)*double(prefs->getOptionToFloat("beta_init_real"))+
    complex_p(0,1)*double(prefs->getOptionToFloat("beta_init_imag"));
    simData->beta_init_sigma_val = prefs->getOptionToFloat("beta_init_sigma");
    
    // The 2D lattice is vectorized in a nx*ny line
    simData->U = InitMatrix(nx, ny, simData->U_val);
    simData->F = InitMatrix(nx, ny, simData->F_val);
    simData->omega = InitMatrix(nx, ny, simData->omega_val);
    simData->gamma = InitMatrix(nx, ny, simData->gamma_val);
    simData->beta_init = InitMatrix(simData->nx,simData->ny, simData->beta_init_val);
    
    // Read the 2D noise, if they exist;
    float_p* deltaUTmp = prefs->getValueOrMatrixReal("disorderU", nx, ny);
    float_p* deltaOmegaTmp = prefs->getValueOrMatrixReal("disorderOmega", nx, ny);
    float_p* deltaGammaTmp = prefs->getValueOrMatrixReal("disorderGamma", nx, ny);
    MatrixCXd deltaU = InitMatrix(nx, ny, deltaUTmp); delete[] deltaUTmp;
    MatrixCXd deltaOmega= InitMatrix(nx, ny, deltaOmegaTmp); delete[] deltaOmegaTmp;
    MatrixCXd deltaGamma = InitMatrix(nx, ny, deltaGammaTmp); delete[] deltaGammaTmp;
    
    simData->U = simData->U + deltaU;
    simData->omega = simData->omega + deltaOmega;
    simData->gamma = simData->gamma + deltaGamma;
    
    simData->n_frames = prefs->getOptionToInt("n_frames");
    simData->t_start = prefs->getOptionToFloat("t_start");
    simData->t_end = prefs->getOptionToFloat("t_end");
    
    return simData;
}

void PrintTimesFile(TWMC_Data& dat, string outputPath)
{
    // Initial iteration values
    float_p t = 0;
    int i_step = 0;
    int i_frame = 0;
    int frame_steps = floor(dat.dt_obs/dat.dt);
    
    std::ofstream tfile, ffile;
    tfile.open(outputPath+folderSeparator+"_times.dat", std::ofstream::out);// | std::ofstream::app );
    while (t<dat.t_end)
    {
        // Print the data
        if((i_step % frame_steps ==0 ) && i_frame < dat.n_frames)
        {
            i_frame = i_frame + 1;
            tfile << t << endl;
        }
        t += dat.dt;
        i_step++;
    }
    tfile.close();
}

void usage(const char * pname)
{
    std::cerr << "Usage: " << pname << std::endl;
    std::cerr << "[-i \"nomefile\"] input file name" << std::endl;
    std::cerr << "[-o \"nomefile\"] output folder name" << std::endl;
}

#include <sys/stat.h>
uint file_exist(const string fileName)
{
    struct stat st;
    stat(fileName.c_str(), &st);
    bool isDir = S_ISDIR(st.st_mode);
    if (isDir)
        return 2;
    else
    {
        std::ifstream infile(fileName);
        return infile.good();
    }
}

float_p* CreateRealVector(int dim, float_p val)
{
    float_p* vec = new float_p[dim];
    for (int i = 0; i!= dim; i++)
    {
        vec[i] = val;
    }
    return vec;
}

complex_p* CreateComplexVector(int dim, complex_p val)
{
    complex_p* vec = new complex_p[dim];
    for (int i = 0; i!= dim; i++)
    {
        vec[i] = val;
    }
    return vec;
}

void SumVectorOnFirst(complex_p* v1, complex_p* v2, int dim)
{
    for(size_t i=0; i!= dim; i++)
    {
        v1[i] = v1[i] + v2[i];
    }
}

void SumVectorOnFirst(float_p* v1, float_p* v2, int dim)
{
    for(size_t i=0; i!= dim; i++)
    {
        v1[i] = v1[i] + v2[i];
    }
}

complex_p* RealToComplexArray(int dim, float_p* arr)
{
    complex_p * result = new complex_p(dim);
    for (size_t i=0; i!= dim; i++)
    {
        result[i] = complex_p(arr[i],0.0);
    }
    return result;
}


#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

// Recursive mkdir
int mkpath(char* file_path, mode_t mode) {
    assert(file_path && *file_path);
    char* p;
    for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
        *p='\0';
        if (mkdir(file_path, mode)==-1) {
            if (errno!=EEXIST) { *p='/'; return -1; }
        }
        *p='/';
    }
    return 0;
}
int mkpathAll(const char* file_path) ;

int mkpathAll(const char* _file_path) {
    char file_path[1500];
    strcpy(file_path,_file_path);
    
    assert(file_path && *file_path);
    char* p;
    for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
        *p='\0';
        if (mkdir(file_path, 0777)==-1) {
            if (errno!=EEXIST) { *p='/'; return -1; }
        }
        *p='/';
    }
    return 0;
}

std::vector<string> findAllFolders(string path)
{
    std::vector<string> result = std::vector<string>(0);
    
    const char* PATH = path.c_str();
    
    DIR *dir = opendir(PATH);
    struct dirent *entry = readdir(dir);
    
    while (entry != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            printf("%s\n", entry->d_name);
            result.push_back(entry->d_name);
        }
        
        entry = readdir(dir);
    }
    closedir(dir);
    return result;
}

std::vector<string> findFoldersContainingString(string path, string pattern)
{
    std::vector<string> allFolders = findAllFolders(path);
    for (int i=0; i< allFolders.size(); i++)
    {
        if (allFolders[i].find(pattern) == std::string::npos)
        {
            allFolders.erase(allFolders.begin()+i);
            i--;
        }
    }
    return allFolders;
}

void CheckCreateFolder(string folderName)
{
    struct stat sb;
    if (!(stat(folderName.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)))
    {
        mkpathAll(folderName.c_str());
    }
}


void CopyFileToFolder(string inputFile, string outputFolder)
{
    std::ifstream  src(inputFile, std::ios::binary);
    std::ofstream  dst(outputFolder+folderSeparator+paramsFileName,   std::ios::binary);
    
    dst << src.rdbuf();
    src.close();
    dst.close();
}