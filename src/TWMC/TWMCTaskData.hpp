//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCSIMTASKDATA_HPP
#define SIMULATOR_TWMCSIMTASKDATA_HPP

#include "Base/TaskData.hpp"

#include "TWMCSystemData.hpp"

#ifdef MPI_SUPPORT
#include <boost/serialization/vector.hpp>
#endif


class TWMCTaskData : public TaskData
{
public:

	TWMCTaskData();
	TWMCSystemData* systemData;

	float_p t_start;
	float_p t_end;

	enum InitialConditions {ReadFromSettings,
		FixedPoint};

	InitialConditions initialCondition;
	unsigned int rngSeed;

protected:

private:
#ifdef MPI_SUPPORT
	friend class boost::serialization::access;
#endif

};

#endif //SIMULATOR_TWMCSIMTASKDATA_HPP
