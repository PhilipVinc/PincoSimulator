//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_IRESULTCONSUMER_HPP
#define SIMULATOR_IRESULTCONSUMER_HPP


#include "Libraries/concurrentqueue.h"

#include "../TaskResults.hpp"

#include <vector>

class IResultConsumer {
public:
	// Inherited public methods
	void EnqueueTasks(std::vector<TaskResults*> tasks);
	void EnqueueTasks(std::vector<TaskResults*> tasks, size_t prodID);

	size_t RequestProducerID();

	void ReportProducerTermination(std::vector<size_t> IDs);

protected:

	virtual void ProducerHasBeenTerminated(size_t producerID) {};
	virtual void AllProducersHaveBeenTerminated() = 0;
	// Queue of tasks that are waiting to be executed by a worker.
	// Fed by Manager, Eaten by Workers
	moodycamel::ConcurrentQueue<TaskResults*> enqueuedTasks;
	size_t nEnqueuedTasks = 0;

	// Producers registered with a token. The producer ID is the index
	// used to access the token vector.
	std::vector<moodycamel::ProducerToken> requestedTokens;
	std::vector<size_t> disabledTokens;
private:
};



#endif //SIMULATOR_IRESULTCONSUMER_HPP
