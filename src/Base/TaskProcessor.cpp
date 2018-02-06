//
// Created by Filippo Vicentini on 20/12/2017.
//

#include "TaskProcessor.hpp"

TaskProcessor::TaskProcessor(std::string solverName)  :
		_solverName(solverName)
{

}

void TaskProcessor::SetConsumer(IResultConsumer* consumer) {
	_consumer = consumer;
}
