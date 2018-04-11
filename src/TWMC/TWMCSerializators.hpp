//
// Created by Filippo Vicentini on 22/02/18.
//

#ifndef TWMCSer
#define TWMCSer
#include "TWMCResults.hpp"
#include "TWMCSystemData.hpp"
#include "TWMCTaskData.hpp"

#include <iostream>

#include "easylogging++.h"

#include "Base/Serialization/SerializationArchiveFormats.hpp"

#include <Libraries/eigen_cereal_serialization.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/array.hpp>


using namespace std;


namespace cereal {

    template<class Archive>
    void serialize(Archive & ar, TWMCSystemData & g)
	{
		LOG(INFO) << "Serializing TWMCSystemData."<< endl;
		ar(g.latticeName);
		ar(g.nx);
		ar(g.ny);
		ar(g.nxy);
		ar(g.cellSz);
		ar(g.F);
		ar(g.U);
		ar(g.omega);
		ar(g.E);
		ar(g.J);
		ar(g.gamma);
		ar(g.beta_init);
		ar(g.J_val);
		ar(g.detuning);
		ar(g.J_AB_val);
		ar(g.J_BC_val);
		ar(g.gamma_val);
		ar(g.beta_init_val);
		ar(g.beta_init_sigma_val);
		ar(g.t_start);
		ar(g.t_end);
		ar(g.dt);
		ar(g.dt_obs);
		ar(g.n_dt);
		ar(g.n_frames);
		ar(g.frame_steps);
		ar(g.PBC);
		ar(g.dimension);
	}

    template<class Archive>
    void save(Archive & ar, TWMCTaskData const & g)
    {
        //cout << "Saving TWMCTask" << endl;
        ar(cereal::virtual_base_class<TaskData>(&g));
        ar(g.t_start);
        ar(g.t_end);
        ar(g.initialCondition);
        ar(g.rngSeed);
        ar(g.systemData);
        ar(g.storeInitialState);
    }

    template<class Archive>
    void load(Archive & ar, TWMCTaskData & g)
    {
        //cout << "Loading TWMCTask" << endl;

        ar(cereal::virtual_base_class<TaskData>(&g));
        ar(g.t_start);
        ar(g.t_end);
        ar(g.initialCondition);
        ar(g.rngSeed);
        ar(g.systemData);
        ar(g.storeInitialState);
        //cout << "Done TWMCTask" << endl;
    }

}

namespace cereal {
	template <class Archive>
	struct specialize<Archive, TWMCTaskData, cereal::specialization::non_member_load_save > {};
} // namespace ...


namespace dummy {
    struct bogus {
        static int bogus_method();
    };
}

// force linking with util.cc
static int bogus_variable = dummy::bogus::bogus_method();


CEREAL_FORCE_DYNAMIC_INIT(TWMC)
#endif