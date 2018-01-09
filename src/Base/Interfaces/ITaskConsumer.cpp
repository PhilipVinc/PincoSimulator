//
// Created by Filippo Vicentini on 22/12/17.
//

#import "ITaskConsumer.hpp"

#import "../TaskData.hpp"

void ITaskConsumer::EnqueueTasks(std::vector < TaskData * > tasks) {
	enqueuedTasks.enqueue_bulk(tasks.begin(), tasks.size());
	nEnqueuedTasks += tasks.size();
}

void ITaskConsumer::EnqueueTasks(std::vector<TaskData *> tasks,
                                       size_t prodID){
	enqueuedTasks.enqueue_bulk(requestedTokens[prodID],
	                           tasks.begin(),
	                           tasks.size());
	nEnqueuedTasks += tasks.size();
}

size_t ITaskConsumer::RequestProducerID() {
	auto i = requestedTokens.size();
	requestedTokens.push_back(moodycamel::ProducerToken(enqueuedTasks));
	return i;
}