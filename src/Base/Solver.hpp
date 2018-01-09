//
// Created by Filippo Vicentini on 20/12/2017.
//

#ifndef SIMULATOR_SOLVER_HPP
#define SIMULATOR_SOLVER_HPP

#include "Libraries/TStaticFactory.h"

class TaskData;
class TaskResults;
class SolverManager;

#include <vector>

class Solver {
public:
	//Solver();
	virtual ~Solver() {};

    virtual void Setup() {};
    virtual std::vector<TaskResults*> Compute(const std::vector<TaskData*> tasks) = 0;

	virtual float ApproximateComputationProgress() const {return 0.0;} ;
	size_t nTasksToRequest = 1;
protected:
    //SolverManager* manager;

private:

};

typedef Base::TFactory<std::string, Solver> SolverFactory;

#endif //SIMULATOR_SOLVER_HPP
