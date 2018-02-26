//
// Created by Filippo Vicentini on 22/02/18.
//

#ifndef TWMCSer
#define TWMCSer


#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/mpi/packed_iarchive.hpp>
#include <boost/mpi/packed_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

// I have to include them because they include the mpi archive
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "TWMCResults.hpp"
#include "TWMCSystemData.hpp"
#include "TWMCTaskData.hpp"
#include <iostream>

using namespace std;

namespace boost {
namespace serialization {

    template<class Archive>
    void serialize(Archive &ar, TWMCSystemData &g, const unsigned int version)
    {
        //cout << "Serializing TWMCSystemData."<< endl;
        ar & g.latticeName;
        ar & g.nx;
        ar & g.ny;
        ar & g.nxy;
        ar & g.cellSz;
        ar & g.F;
        ar & g.U;
        ar & g.omega;
        ar & g.E;
        ar & g.J;
        ar & g.gamma;
        ar & g.beta_init;
        ar & g.J_val;
        ar & g.detuning;
        ar & g.J_AB_val;
        ar & g.J_BC_val;
        ar & g.gamma_val;
        ar & g.beta_init_val;
        ar & g.beta_init_sigma_val;
        ar & g.t_start;
        ar & g.t_end;
        ar & g.dt;
        ar & g.dt_obs;
        ar & g.n_dt;
        ar & g.n_frames;
        ar & g.frame_steps;
        ar & g.PBC;
        ar & g.dimension;

        //cout << "Finished serializing TWMCSystemData."<< endl;
    }

    template<class Archive>
    void serialize(Archive & ar, TWMCTaskData & g, const unsigned int version)
    {
        //cout << "Serializing TWMCTaskData."<< endl;
        ar & boost::serialization::base_object<TaskData>(g);
        ar & g.t_start;
        ar & g.t_end;
        ar & g.initialCondition;
        ar & g.rngSeed;
        ar & g.systemData;

        //cout << "Finished serializing TWMCTaskData."<< endl;
    }


    template<class Archive>
    void serialize(Archive & ar, TWMCResults & g, const unsigned int version)
    {
        //cout << "Serializing TWMCResults."<< endl;
        ar & g.beta_t;
        ar & g.complexMatrices;
        ar & g.realMatrices;

        ar & g.work_t;
        ar & g.area_t;

        ar & g.frames;
        ar & g.extraDataMemory[0];
        ar & g.extraDataMemory[1];

        ar & boost::serialization::base_object<TaskResults>(g);
        //cout << "Finished serializing TWMCResults."<< endl;
    };

} // namespace serialization
} // namespace boost

namespace dummy {
    struct bogus {
        static int bogus_method();
    };
}

// force linking with util.cc
static int bogus_variable = dummy::bogus::bogus_method();


#endif