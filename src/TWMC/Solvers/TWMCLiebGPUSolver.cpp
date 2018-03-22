//
// Created by Filippo Vicentini on 26/12/17.
//

#ifdef GPU_SUPPORT

#include "TWMCLiebGPUSolver.hpp"

#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"
#include "TWMC/TWMCResults.hpp"

#include "Base/NoisyMatrix.hpp"
#include "Base/Utils/EigenUtils.hpp"

#include <iostream>
#include <random>
#include <algorithm>
#include <functional>

using namespace std;


// Local Utility Methods
inline void randCMat(MatrixCXd *mat, std::mt19937 &gen, std::normal_distribution<> norm);
inline complex_p randC(std::mt19937 &gen, std::normal_distribution<> norm);

TWMCLiebGPUSolver::TWMCLiebGPUSolver() :
        ctx(vex::Filter::GPU && vex::Filter::Env && vex::Filter::Count(1)),
        iJ_GPU(ctx.queue()[0])
{
    nTasksToRequest = 5000;
}

TWMCLiebGPUSolver::~TWMCLiebGPUSolver() {

}

void TWMCLiebGPUSolver::SetupGPU(size_t gpuId)
{
    cout << "Initialized GPU Context #" << gpuId << endl;
    ctx = vex::Context(vex::Filter::GPU && vex::Filter::Env && vex::Filter::Count(1)) ;//&& vex::Filter::Position(gpuId) );
    iJ_GPU = vex::sparse::matrix<complex_p>(ctx.queue()[0]);

    cout << "--number of devices:" << ctx.size() << endl;

}

void TWMCLiebGPUSolver::SuggestBatchSize() {

}


//GPU STUFF
namespace vex {

    template <typename T>
    struct is_cl_native< std::complex<T> > : std::true_type {};

    template <typename T>
    struct type_name_impl< std::complex<T> >
    {
        static std::string get() {
            std::ostringstream s;
            s << type_name<T>() << "2";
            return s.str();
        }
    };

    template <typename T>
    struct cl_scalar_of< std::complex<T> > {
        typedef T type;
    };

} // namespace vex

// Now we specialize a template from <vexcl/sparse/spmv_ops.hpp> that allows
// vexcl to generate semantically correct smpv kernels for complex values:
namespace vex {
    namespace sparse {

        template <typename T>
        struct spmv_ops_impl<std::complex<T>, std::complex<T>>
        {
            static void decl_accum_var(backend::source_generator &src, const std::string &name)
            {
                src.new_line() << type_name<T>() << "2 " << name << " = {0,0};";
            }

            static void append(backend::source_generator &src,
                               const std::string &sum, const std::string &val)
            {
                src.new_line() << sum << " += " << val << ";";
            }

            static void append_product(backend::source_generator &src,
                                       const std::string &sum, const std::string &mat_val, const std::string &vec_val)
            {
                src.new_line() << sum << ".x += "
                               << mat_val << ".x * " << vec_val << ".x - "
                               << mat_val << ".y * " << vec_val << ".y;";
                src.new_line() << sum << ".y += "
                               << mat_val << ".x * " << vec_val << ".y + "
                               << mat_val << ".y * " << vec_val << ".x;";
            }
        };

    } // namespace sparse
} // namespace vex


//---------------------------------------------------------------------------
// Complex multiplication:
//---------------------------------------------------------------------------
VEX_FUNCTION(cl_double2, cmul, (cl_double2, a)(cl_double2, b),
             double2 r = {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x};
                     return r;
);

VEX_FUNCTION(cl_double2, rcmul, (cl_double, a)(cl_double2, b),
             double2 r = {a* b.x, a*b.x};
                     return r;
);

//---------------------------------------------------------------------------
// Complex division:
//---------------------------------------------------------------------------
VEX_FUNCTION(cl_double2, cdiv, (cl_double2, a)(cl_double2, b),
             double d = b.x * b.x + b.y * b.y;
                     double2 r = {(a.x * b.x + a.y * b.y) / d, (a.y * b.x - a.x * b.y) / d};
                     return r;
);

VEX_FUNCTION(cl_double2, cabs2, (cl_double2, a),
             double2 r = {(a.x * a.x + a.y * a.y), 0.0};
                     return r;
);


void TWMCLiebGPUSolver::Setup() {
    const TWMCSystemData* data = lastSharedSystemData;

    nx = data->nx;
    ny = data->ny;

    size_t nMatTot = nx*ny*cellSz*nTasksToRequest;
    size_t szSingle = nx*ny*cellSz*sizeof(complex_p);

    if (data->dimension == TWMCSystemData::Dimension::D1 || data->dimension == TWMCSystemData::Dimension::D0 )
    {
        // Temporary variables, for now ny = 1;
        temp_gamma = sqrt(data->gamma_val*data->dt/4.0);
        tmpRand = MatrixCXd::Zero(data->nx, nTasksToRequest*cellSz*data->ny);
        kai_t = MatrixCXd::Zero(data->nx, nTasksToRequest*data->ny);
        a_t = MatrixCXd::Zero(data->nx, nTasksToRequest*cellSz*data->ny);

        // Merge the three variants in a single for Delta
        delta =  -data->E->GenerateNoNoise().array() + data->detuning ;

        // For U
        U = ij*data->U->GenerateNoNoise();
        F = ij*data->F->GenerateNoNoise();
        MatrixCXd real_step_linear_single = (-ij*delta.array() - data->gamma_val/2.0);


        std::vector<Eigen::Triplet<complex_p>> elList;
        for (int j=0; j != nTasksToRequest; j++) {
            auto offset = cellSz*ny*j;
            for (int i = 0; i != data->ny; i++) {
                // Internal coupling
                // AB
                elList.push_back(Eigen::Triplet<complex_p>(offset + 3 * i, offset +3 * i + 1, data->J_AB_val));
                elList.push_back(Eigen::Triplet<complex_p>(offset + 3 * i + 1, offset + 3 * i, data->J_AB_val));
                // BC
                elList.push_back(Eigen::Triplet<complex_p>(offset + 3 * i + 1, offset + 3 * i + 2, data->J_BC_val));
                elList.push_back(Eigen::Triplet<complex_p>(offset + 3 * i + 2, offset + 3 * i + 1, data->J_BC_val));

                // Inter-Site Coupling BC. Skip for last element
                if (i == data->ny - 1) {
                    // End of chain. If there are PBC, put them. Otherwise stop.
                    if (data->PBC) {
                        elList.push_back(
                                Eigen::Triplet<complex_p>(offset + 3 * i + 2, offset + 0 + 1, data->J_BC_val));
                        elList.push_back(
                                Eigen::Triplet<complex_p>(offset + 0 + 1, offset + 3 * i + 2, data->J_BC_val));
                    }
                    break;
                }

                elList.push_back(
                        Eigen::Triplet<complex_p>(offset + 3 * i + 2, offset + 3 * (i + 1) + 1, data->J_BC_val));
                elList.push_back(
                        Eigen::Triplet<complex_p>(offset + 3 * (i + 1) + 1, offset + 3 * i + 2, data->J_BC_val));
            }
        }
        coupling_mat = SparseCXd(cellSz*ny*nTasksToRequest,cellSz*ny*nTasksToRequest);
        coupling_mat.setFromTriplets(elList.begin(), elList.end());

        // Copy to tmp arrays
        complexU = MatrixCXd::Zero(nx, ny*cellSz*nTasksToRequest);
        iF = MatrixCXd::Zero(nx, ny*cellSz*nTasksToRequest);
        real_step_linear = MatrixCXd::Zero(nx, ny*cellSz*nTasksToRequest);
        complexId = InitMatrix(nx, ny*cellSz*nTasksToRequest, complex_p(1.0,0.0));

        for (int i=0; i < nTasksToRequest; i++)
        {
            std::memcpy(&(complexU.data()[i*U.size()]), U.data(), szSingle );
            std::memcpy(&(iF.data()[i*F.size()]), F.data(), szSingle );
            std::memcpy(&(real_step_linear.data()[i*real_step_linear_single.size()]), real_step_linear_single.data(), szSingle );
        }

        coupling_mat.makeCompressed();
        auto outerId = coupling_mat.outerIndexPtr();
        auto innerId = coupling_mat.innerIndexPtr();
        auto valId = coupling_mat.valuePtr();
        std::vector<int> ptr(outerId, outerId+coupling_mat.nonZeros());
        std::vector<int> col(innerId, innerId+coupling_mat.rows()+1);
        std::vector<complex_p> val(valId, valId+coupling_mat.nonZeros());

       // cout << coupling_mat;

        iJ_GPU = vex::sparse::matrix<complex_p>(ctx, coupling_mat.rows(), coupling_mat.cols(),
               //coupling_mat.outerIndexPtr(), coupling_mat.innerIndexPtr(),
                //coupling_mat.valuePtr());
        ptr, col, val);

        real_step_linear_GPU = vex::vector<complex_p>(ctx, real_step_linear.size(),
                                                      real_step_linear.data());
        complexU_GPU = vex::vector<complex_p>(ctx, complexU.size(),
                                              complexU.data());
        iF_GPU=vex::vector<complex_p>(ctx, iF.size(), iF.data());
        complexId_GPU = vex::vector<complex_p>(ctx, complexId.size(), complexId.data());

        beta_t = MatrixCXd(nx, cellSz*ny*nTasksToRequest);
        beta_t_GPU = vex::vector<complex_p>(ctx, complexId.size());
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
std::vector<std::unique_ptr<TaskResults>> TWMCLiebGPUSolver::Compute(const std::vector<TaskData *> tasks)
{
    // Profiling
    completedTasks = 0;
    taskBatch = 0;

    //cout << "computing " << tasks.size() <<endl;
    std::vector<TWMCResults*> allResults;
    std::vector<TaskResults*> allResultsBase;
    std::vector<size_t> seeds(tasks.size(), 0);

    if (tasks.size() == 0)
        return allResultsBase;

    size_t taskI = 0;
    while (taskI < tasks.size())
    {
        TWMCTaskData *task = static_cast<TWMCTaskData *>(tasks[taskI]);

        if (task->systemData != lastSharedSystemData) {

            lastSharedSystemData = task->systemData;
            Setup();
        }

        size_t maxTaskI = taskI;
        for (; maxTaskI < tasks.size(); maxTaskI++ ) {
            TWMCTaskData *task = static_cast<TWMCTaskData *>(tasks[maxTaskI]);
            TWMCResults *res = new TWMCResults(task);
            allResults.push_back(res);
            allResultsBase.push_back(res);
            seeds[maxTaskI] = task->rngSeed;
            res->SetId(seeds[maxTaskI]);

            // Check if the system is always the same
            if (task->systemData != lastSharedSystemData) {
                break;
            }
        }
        taskBatch = maxTaskI-taskI;

        // Setup the random number generation
        std::mt19937 gen(seeds[taskI]);
        std::normal_distribution<> normal(0, 1); // mean = 0, std = 1;

        const TWMCSystemData* data = lastSharedSystemData;

        // Generate noisy Matrices
        bool updateMats = false;
        size_t noiseN = 0;
        if (data->U->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats = true;
            for (int i=taskI; i < maxTaskI; i++) {
                U = ij * data->U->Generate(gen);
                std::memcpy(&(complexU.data()[i * U.size()]), U.data(), nx*ny*3*sizeof(complex_p));

                // And now store this matrix in the results
                complex_p *resData = allResults[i]->complexMatrices[noiseN];
                complex_p *matData = U.data();

                for (unsigned j = 0; j < data->nxy; j++) {
                    resData[j] = matData[j];
                }
            }
            noiseN++;
        }
        if (data->E->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            for (int i=taskI; i < maxTaskI; i++) {

                E = data->omega->Generate(gen);

                delta =  -data->E->GenerateNoNoise().array() + data->detuning ;
                MatrixCXd real_step_linear_single = (-ij*delta.array() - data->gamma_val/2.0);
                std::memcpy(&(real_step_linear.data()[i*real_step_linear_single.size()]), real_step_linear_single.data(), nx*ny*3*sizeof(complex_p) );

                // And now store this matrix in the results
                complex_p *resData = allResults[i]->complexMatrices[noiseN];
                complex_p *matData = E.data();

                for (unsigned j = 0; j < data->nxy; j++) {
                    resData[j] = matData[j];
                }
            }
            noiseN++;
        }
        if (data->F->GetNoiseType() != NoisyMatrix::NoiseType::None)
        {
            updateMats=true;
            for (int i=taskI; i < maxTaskI; i++) {

                F = data->F->Generate(gen);
                std::memcpy(&(iF.data()[i * F.size()]), F.data(), nx*ny*3*sizeof(complex_p));

                // And now store this matrix in the results
                complex_p *resData = allResults[i]->complexMatrices[noiseN];
                complex_p *matData = F.data();

                for (unsigned j = 0; j < data->nxy; j++) {
                    resData[j] = matData[j];
                }
            }
            noiseN++;
        }
        // End handling of noise matrices

        auto initialCondition = TWMCTaskData::InitialConditions::ReadFromSettings;
        for (int i=0; i < nTasksToRequest; i++)
        {
            std::memcpy(&(beta_t.data()[i*data->beta_init.size()]),
                   data->beta_init.data(), nx*ny*3*sizeof(complex_p));
        }

        switch (initialCondition) {
            case TWMCTaskData::InitialConditions::ReadFromSettings:
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
        // Copy the starting beta_t to the gpu
        vex::copy(beta_t.data(), beta_t.data() + cellSz*data->nxy*maxTaskI,beta_t_GPU.begin());

        t_start = data->t_start;
        t_end = data->t_end;

        int i_step = 0;
        int i_frame = 0;
        //int frame_steps = floor(data->dt_obs/data->dt);

        // Initialize the beta value to the starting value.

        auto dt4 = sqrt(data->gamma_val * data->dt / 4.0);
        auto dt = data->dt;

        using vex::tag;
        vex::RandomNormal<cl_double2> rnd;

        while (t <= data->t_end) {
            // Evolve
            beta_t_GPU = vex::tag<1>(beta_t_GPU) + (cmul( real_step_linear_GPU + cmul(complexU_GPU,(cabs2(vex::tag<1>(beta_t_GPU))-std::complex<double>(1.0,0.0))) ,vex::tag<1>(beta_t_GPU) )
                                                    - iJ_GPU*vex::tag<1>(beta_t_GPU) +iF_GPU)*dt + dt4*rnd(vex::element_index(0, nx), std::rand());

            // Print the data
            if ((i_step % data->frame_steps == 0) && i_frame < data->n_frames)
            {
                //cout << "beta_t_gpu_negin: "<<beta_t_GPU.begin() << endl;
                //cout << "beta_t_gpu_end: "<<beta_t_GPU.end() << endl;
                auto cc = beta_t_GPU.end();
                auto dd = beta_t_GPU.begin();
                auto kk = cc-dd;

                vex::copy(beta_t_GPU.begin(), beta_t_GPU.end(), beta_t.data());

                for (int i=taskI; i < maxTaskI; i++) {
                    size_t size = nx * ny * 3;
                    complex_p *data = beta_t.data() + i*size;

                    std::memcpy(&(allResults[i]->beta_t[i_frame*size]), data, sizeof(complex_p)*size);

                    /*for (unsigned j = 0; j < size; j++) {
                        allResults[i]->beta_t[i_frame * size + j] = data[j];
                    }*/
                }
                i_frame = i_frame + 1;
            }
            t += data->dt;
            i_step++;
        }

        taskI = taskI + maxTaskI;
        completedTasks = taskI;
    }
    completedTasks = 0;
    taskBatch = 0;
    return allResultsBase;
};


float TWMCLiebGPUSolver::ApproximateComputationProgress() const
{
    return completedTasks + taskBatch*float((t-t_start)/t_end);
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

#endif
