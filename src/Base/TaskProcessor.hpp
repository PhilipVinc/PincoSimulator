//
// Created by Filippo Vicentini on 20/12/2017.
//

#ifndef SIMULATOR_TASKMANAGER_HPP
#define SIMULATOR_TASKMANAGER_HPP

#include "Interfaces/ITaskConsumer.hpp"
#include "Interfaces/IResultConsumer.hpp"

#include "Settings.hpp"

#include <memory>
#include <string>

//class Settings;
class TaskData;
class Solver;


class TaskProcessor : public ITaskConsumer
{
public:
	TaskProcessor(std::string solverName);
    virtual ~TaskProcessor() {};

	virtual void Setup() = 0;
	virtual void Update() = 0;

	virtual void AllProducersHaveBeenTerminated() = 0;
	void SetConsumer(std::shared_ptr<IResultConsumer> consumer);
	// Missing: some way to feed forward the results to the

	virtual size_t NumberOfCompletedTasks() const {return 0;} ;
	virtual float Progress() const { return 0;};


protected:
  const std::string _solverName;

	std::shared_ptr<IResultConsumer> _consumer;
private:

};


#endif //SIMULATOR_TASKMANAGER_HPP
