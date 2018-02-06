//
// Created by Filippo Vicentini on 20/12/2017.
//

#ifndef SIMULATOR_TASKMANAGER_HPP
#define SIMULATOR_TASKMANAGER_HPP

#import "Interfaces/ITaskConsumer.hpp"
#import "Interfaces/IResultConsumer.hpp"

#import "Settings.hpp"

#import <string>

//class Settings;
class TaskData;
class Solver;


class TaskProcessor : public ITaskConsumer
{
public:
	TaskProcessor(std::string solverName);
    //~TaskProcessor();

	virtual void Update() = 0;

	void SetConsumer(IResultConsumer* consumer);
	// Missing: some way to feed forward the results to the
protected:
    const std::string _solverName;

	IResultConsumer* _consumer;
private:

};


#endif //SIMULATOR_TASKMANAGER_HPP
