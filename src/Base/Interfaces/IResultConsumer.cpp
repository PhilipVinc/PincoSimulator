//
// Created by Filippo Vicentini on 22/12/17.
//

#include "IResultConsumer.hpp"

#import "../TaskResults.hpp"

void IResultConsumer::EnqueueTasks(std::vector <TaskResults *> tasks) {
	enqueuedTasks.enqueue_bulk(tasks.begin(), tasks.size());
	nEnqueuedTasks += tasks.size();
}

void IResultConsumer::EnqueueTasks(std::vector<TaskResults *> tasks,
                                       size_t prodID){
	enqueuedTasks.enqueue_bulk(requestedTokens[prodID],
	                           tasks.begin(),
	                           tasks.size());
	nEnqueuedTasks += tasks.size();
}

size_t IResultConsumer::RequestProducerID() {
	auto i = requestedTokens.size();
	requestedTokens.push_back(moodycamel::ProducerToken(enqueuedTasks));
	return i;
}

void IResultConsumer::ReportProducerTermination(std::vector<size_t> IDs) {
	for (size_t i = 0; i < IDs.size(); i++)
	{
		disabledTokens.push_back(IDs[i]);
	}

	if (disabledTokens.size() == requestedTokens.size())
		AllProducersHaveBeenTerminated();
}