//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCSIMTASKDATA_HPP
#define SIMULATOR_TWMCSIMTASKDATA_HPP

#include "Base/TaskData.hpp"

#include "TWMCSystemData.hpp"

#ifdef MPI_SUPPORT
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

#endif


class TWMCTaskData : public TaskData
{
public:
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
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<TaskData>(*this);
		ar & t_start;
		ar & t_end;
		ar & initialCondition;
		ar & rngSeed;
		ar & systemData;
	}
#endif

};

#ifdef MPI_SUPPORT
    BOOST_CLASS_EXPORT_KEY2(TWMCTaskData, "TWMCTaskData")
#endif


#endif //SIMULATOR_TWMCSIMTASKDATA_HPP
