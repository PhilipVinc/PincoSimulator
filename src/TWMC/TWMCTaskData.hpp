//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCSIMTASKDATA_HPP
#define SIMULATOR_TWMCSIMTASKDATA_HPP

#include "Base/TaskData.hpp"

#include "TWMCSystemData.hpp"

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

};


#endif //SIMULATOR_TWMCSIMTASKDATA_HPP
