//
// Created by Filippo Vicentini on 27/11/2017.
//

#include "TWMCLiebSolver.hpp"

#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"
#include "TWMC/TWMCResults.hpp"

#include "Base/NoisyMatrix.hpp"

#include <iostream>
#include <random>
#include <algorithm>
#include <functional>


using namespace std;


// Local Utility Methods
inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm);

TWMCLiebSolver::TWMCLiebSolver() {
    nTasksToRequest = 1;
}

TWMCLiebSolver::~TWMCLiebSolver() {

}

void TWMCLiebSolver::Setup() {
    const TWMCSystemData* data = lastSharedSystemData;

    nx = data->nx;
    ny = data->ny;

    if (data->dimension == TWMCSystemData::Dimension::D1 || data->dimension == TWMCSystemData::Dimension::D0 )
    {
        // Temporary variables, for now ny = 1;
        temp_gamma = sqrt(data->gamma_val*data->dt/4.0);
        tmpRand = MatrixCXd::Zero(data->nx, 3*data->ny);
        kai_t = MatrixCXd::Zero(data->nx, data->ny);
        a_t = MatrixCXd::Zero(data->nx, 3*data->ny);

        // Merge the three variants in a single for Delta
        delta =  -data->E->GenerateNoNoise().array() + data->detuning ;

        // For U
        U = data->U->GenerateNoNoise();
        F = data->F->GenerateNoNoise();

        real_step_linear = (-ij*delta.array() - data->gamma_val/2.0);

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

        for (int i =0; i!= cellSz*data->ny; i++)
        {
            elList.push_back(Eigen::Triplet<complex_p>(i, i, -real_step_linear(i)/ij));
        }
        coupling_mat_total = SparseCXd(cellSz*ny,cellSz*ny);
        coupling_mat_total.setFromTriplets(elList.begin(), elList.end());

        //cout << coupling_mat_total << endl;

    }
    else if (data->dimension == TWMCSystemData::Dimension::D2)
    {
        std::cerr << "ERROR: 2D Not Supported for LIEB "<<endl;
    }

    // If we have a 1D system, then we put to 0 it's contribution of the cosinus (1D/2D) code.
    double flag1DNx = (data->nx==1) ? 0.0 : 1.0;
    double flag1DNy = (data->ny==1) ? 0.0 : 1.0;
}


//
// The Truncated Wigner Evolution Method for a Lieb Lattice.
//
std::vector<TaskResults*> TWMCLiebSolver::Compute(const std::vector<TaskData *> tasks)
{
    //cout << "computing" <<endl;
    std::vector<TaskResults*> allResults;
    for (size_t i =0; i < tasks.size(); i++ ) {
        TWMCTaskData *task = static_cast<TWMCTaskData *>(tasks[i]);

        // Check if the system is always the same
        if (task->systemData != lastSharedSystemData) {
            lastSharedSystemData = task->systemData;
            Setup();
        }

        // Setup the single simulation
        TWMCResults *res = new TWMCResults(task);
        allResults.push_back(res);
        unsigned int seed = task->rngSeed;
        res->SetId(seed);
        auto initialCondition = TWMCTaskData::InitialConditions::ReadFromSettings;

        // Setup the random number generation
        std::mt19937 gen(seed);
        std::normal_distribution<> normal(0, 1); // mean = 0, std = 1;

        const TWMCSystemData* data = lastSharedSystemData;

        // Generate noisy Matrices
        bool updateMats = false;
        size_t noiseN = 0;
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            U = data->U->Generate(gen);

            // And now store this matrix in the results
            complex_p* resData = res->complexMatrices[noiseN]; noiseN++;
            complex_p* matData = U.data();

            for (unsigned j= 0; j < data->nxy; j++)
            {
                resData[j] = matData[j];
            }
        }
        if (data->E->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            E = data->omega->Generate(gen);

            // And now store this matrix in the results
            complex_p* resData = res->complexMatrices[noiseN]; noiseN++;
            complex_p* matData = E.data();

            for (unsigned j= 0; j < data->nxy; j++)
            {
                resData[j] = matData[j];
            }
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            F = data->F->Generate(gen);

            // And now store this matrix in the results
            complex_p* resData = res->complexMatrices[noiseN]; noiseN++;
            complex_p* matData = F.data();

            for (unsigned j= 0; j < data->nxy; j++)
            {
                resData[j] = matData[j];
            }
        }
        if (updateMats)
        {
            delta =  -data->E->GenerateNoNoise().array() + data->detuning ;
        }
        // End handling of noise matrices


        MatrixCXd beta_t = MatrixCXd(nx, 3*ny);
        switch (initialCondition) {
            case TWMCTaskData::InitialConditions::ReadFromSettings:
                beta_t = data->beta_init;
                randCMat(&tmpRand, gen, normal);
                if (data->beta_init_sigma_val != 0) {
                    beta_t += data->beta_init_sigma_val * tmpRand;
                } else {
                    beta_t += temp_gamma * tmpRand;
                }
                t = data->t_start;
                break;
            case TWMCTaskData::InitialConditions::FixedPoint:
                beta_t = beta_t_init;
                break;
            default:
                break;
        }

        t_start = data->t_start;
        t_end = data->t_end;

        int i_step = 0;
        int i_frame = 0;
        //int frame_steps = floor(data->dt_obs/data->dt);

        // Initialize the beta value to the starting value.

        auto dt4 = sqrt(data->gamma_val * data->dt / 4.0);
        auto dt = data->dt;

        while (t <= data->t_end) {
            // Compute the a_t, that is used for the kai in the heun scheme
            a_t = ((real_step_linear.array() + ij * U.array() * (beta_t.array().abs2() - 1.0)) *
                   beta_t.array() - ij * (beta_t * coupling_mat).array() + ij * F.array()) * dt;
            randCMat(&tmpRand, gen, normal);
            kai_t = beta_t.array() + a_t.array() + dt4 * tmpRand.array();

            beta_t = kai_t;

            // Print the data
            if ((i_step % data->frame_steps == 0) && i_frame < data->n_frames) {
                size_t size = res->nx * res->ny * 3;
                complex_p *data = beta_t.data();

                memcpy(&res->beta_t[i_frame*size], data, sizeof(complex_p)*size);
                /*
                for (unsigned j = 0; j < size; j++) {
                    res->beta_t[i_frame * size + j] = data[j];
                }*/
                i_frame = i_frame + 1;
            }
            t += data->dt;
            i_step++;
        }
    }
    return allResults;
};


float TWMCLiebSolver::ApproximateComputationProgress() const
{
    return float((t-t_start)/t_end);
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