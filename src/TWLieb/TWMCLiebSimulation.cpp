//
// Created by Filippo Vicentini on 27/11/2017.
//

#include "TWMCLiebSimulation.hpp"

#include "NoisyMatrix.hpp"
#include "Settings.hpp"
#include "TaskData.hpp"
#include "TWLiebSimData.hpp"
#include "TWMCLiebResults.hpp"

#include <fftw3.h>

#include <iostream>
#include <random>
#include <algorithm>
#include <functional>


using namespace std;


// Local Utility Methods
inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm);

TWMCLiebSimulation::TWMCLiebSimulation(const TWLiebSimData* TaskData)
{
    data = TaskData;
    nx = data->nx;
    ny = data->ny;


    if (TaskData->dimension == TWLiebSimData::Dimension::D1 || TaskData->dimension == TWLiebSimData::Dimension::D0 )
    {
        // Temporary variables, for now ny = 1;
        temp_gamma = sqrt(data->gamma_val*data->dt/4.0);
        tmpRand = MatrixCXd::Zero(data->nx, 3*data->ny);
        kai_t = MatrixCXd::Zero(data->nx, 3*data->ny);
        a_t = MatrixCXd::Zero(data->nx, 3*data->ny);

        // Merge the three variants in a single for Delta
        delta = MatrixCXd::Zero(data->nx, 3*data->ny);

        auto tmpA = data->E_A->GenerateNoNoise();
        auto tmpB = data->E_B->GenerateNoNoise();
        auto tmpC = data->E_C->GenerateNoNoise();
        for (int i=0; i != data->ny; i++)
        {
            delta(3*i) = tmpA(i);
            delta(3*i+1) = tmpB(i);
            delta(3*i+2) = tmpC(i);
        }
        delta = - delta.array() + data->detuning;

        // For U
        U = MatrixCXd::Zero(data->nx, 3*data->ny);

        tmpA = data->U_A->GenerateNoNoise();
        tmpB = data->U_B->GenerateNoNoise();
        tmpC = data->U_C->GenerateNoNoise();
        for (int i=0; i != data->ny; i++)
        {
            U(3*i) = tmpA(i);
            U(3*i+1) = tmpB(i);
            U(3*i+2) = tmpC(i);
        }

        // For F
        F = MatrixCXd::Zero(data->nx, 3*data->ny);

        tmpA = data->F_A->GenerateNoNoise();
        tmpB = data->F_B->GenerateNoNoise();
        tmpC = data->F_C->GenerateNoNoise();
        for (int i=0; i != data->ny; i++)
        {
            F(3*i) = tmpA(i);
            F(3*i+1) = tmpB(i);
            F(3*i+2) = tmpC(i);
        }

        real_step_linear = (-ij*delta.array() - data->gamma_val/2.0);
        //cout << "real_step_linear=" << endl << real_step_linear << endl<<endl;

        coupling_mat = SparseCXd(3*ny,3*ny);

        std::vector<Eigen::Triplet<complex_p>> elList;
        for (int i=0; i != data->ny; i++)
        {
            // Internal coupling
            // AB
            elList.push_back(Eigen::Triplet<complex_p>(3*i,3*i+1,data->J_AB_val));
            elList.push_back(Eigen::Triplet<complex_p>(3*i+1,3*i,data->J_AB_val));
            // BC
            elList.push_back(Eigen::Triplet<complex_p>(3*i+1,3*i+2,data->J_BC_val));
            elList.push_back(Eigen::Triplet<complex_p>(3*i+2,3*i+1,data->J_BC_val));

            // Inter-Site Coupling BC. Skip for last element
            if(i ==data->ny-1)
            {
                // End of chain. If there are PBC, put them. Otherwise stop.
                if (data->PBC)
                {
                    elList.push_back(Eigen::Triplet<complex_p>(3*i+2,0+1,data->J_BC_val));
                    elList.push_back(Eigen::Triplet<complex_p>(0+1,3*i+2,data->J_BC_val));
                }
                break;
            }

            elList.push_back(Eigen::Triplet<complex_p>(3*i+2,3*(i+1)+1,data->J_BC_val));
            elList.push_back(Eigen::Triplet<complex_p>(3*(i+1)+1,3*i+2,data->J_BC_val));
        }
        coupling_mat.setFromTriplets(elList.begin(), elList.end());

        for (int i =0; i!= 3*data->ny; i++)
        {
            elList.push_back(Eigen::Triplet<complex_p>(i, i, -real_step_linear(i)/ij));
        }
        coupling_mat_total = SparseCXd(3*ny,3*ny);
        coupling_mat_total.setFromTriplets(elList.begin(), elList.end());

    }
    else if (TaskData->dimension == TWLiebSimData::Dimension::D2)
    {
        std::cerr << "ERROR: 2D Not Supported for LIEB "<<endl;
    }

    // If we have a 1D system, then we put to 0 it's contribution of the cosinus (1D/2D) code.
    double flag1DNx = (data->nx==1) ? 0.0 : 1.0;
    double flag1DNy = (data->ny==1) ? 0.0 : 1.0;


    // Create structure to hold results;
    res = new TWMCLiebResults(data);
}

TWMCLiebSimulation::~TWMCLiebSimulation()
{
    delete res;
}

void TWMCLiebSimulation::Setup(TaskData* TaskData)
{

}

void TWMCLiebSimulation::Initialize(unsigned int _seed, size_t resultId)
{
    seed = _seed;
    res->SetId(resultId);
    initialCondition = InitialConditions::ReadFromSettings;
}

void TWMCLiebSimulation::Initialize(unsigned int _seed, MatrixCXd beta_init, float_p t0, size_t resultId)
{
    seed = _seed;
    res->SetId(resultId);
    initialCondition = InitialConditions::FixedPoint;
    beta_t_init = beta_init;
    t = t0;
}

//
// The Truncated Wigner Evolution Method for a Lieb Lattice.
//
void TWMCLiebSimulation::Compute()
{
    // Setup the random number generation
    std::mt19937 gen(seed);
    std::normal_distribution<> normal(0,1); // mean = 0, std = 1;

    // Generate noisy Matrices
    bool updateMats = false; size_t noiseN = 0;

    MatrixCXd beta_t = MatrixCXd(3*nx, ny);
    switch (initialCondition)
    {
        case ReadFromSettings:
            beta_t = data->beta_init;
            randCMat(&tmpRand, gen, normal);
            if (data->beta_init_sigma_val != 0)
            {
                beta_t += data->beta_init_sigma_val*tmpRand;
            }
            else
            {
                beta_t += temp_gamma*tmpRand;
            }
            t = data->t_start;
            break;
        case FixedPoint:
            beta_t = beta_t_init;
            break;
        default:
            break;
    }

    int i_step = 0;
    int i_frame = 0;
    //int frame_steps = floor(data->dt_obs/data->dt);

    // Initialize the beta value to the starting value.

    auto dt4 = sqrt(data->gamma_val*data->dt/4.0);
    auto dt = data->dt;

    while (t<=data->t_end)
    {
        // Compute the a_t, that is used for the kai in the heun scheme
        a_t = ((real_step_linear.array() + ij*U.array()*(beta_t.array().abs2()-1.0) )*beta_t.array() -ij*(beta_t*coupling_mat).array() + ij*F.array())*dt;
        randCMat(&tmpRand, gen, normal);
        kai_t = beta_t.array() + a_t.array() + dt4*tmpRand.array();

        beta_t = kai_t;

        // Print the data
        if((i_step % data->frame_steps ==0 ) && i_frame < data->n_frames)
        {
            size_t size = res->nx*res->ny*3;
            complex_p* data = beta_t.data();

            for (unsigned j= 0; j < size; j++)
            {
                res->beta_t[i_frame*size + j] = data[j];
            }
            i_frame = i_frame + 1;
        }
        t += data->dt;
        i_step++;
    }
};

TaskResults* TWMCLiebSimulation::GetResults() const
{
    return res;
}

float TWMCLiebSimulation::ApproximateComputationProgress() const
{
    return (t-data->t_end)/(data->t_end-data->t_start);
}

// ************************************* //
// ****** Local Utilituy Methods  ****** //
// ************************************* //

complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm)
{
    return (norm(gen)+ij*norm(gen));
}

void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm)
{
    complex_p* vals = mat->data();
    size_t dim=mat->rows()*mat->cols();

    for (size_t i =0; i<dim ; i++)
    {
        vals[i] = norm(gen)+ij*norm(gen);
    }
}
