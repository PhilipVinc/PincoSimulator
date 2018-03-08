//
// Created by Filippo Vicentini on 09/01/2018.
//

#ifndef SIMULATOR_SOLVERGPU_HPP
#define SIMULATOR_SOLVERGPU_HPP

#include "Solver.hpp"

#include "Libraries/TStaticFactory.h"

class TaskData;
class TaskResults;
class SolverManager;

#include <vector>

class SolverGPU : public Solver {
public:
	//SolverGPU();
	virtual ~SolverGPU() {};

    virtual void SetupGPU(size_t gpuId) {};
    virtual void SuggestBatchSize() {};
protected:

private:

};

typedef Base::TFactory<std::string, SolverGPU> SolverGPUFactory;


#endif //SIMULATOR_SOLVERGPU_HPP
