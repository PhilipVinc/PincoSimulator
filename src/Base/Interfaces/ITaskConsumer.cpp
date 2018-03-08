//
// Created by Filippo Vicentini on 22/12/17.
//

#import "ITaskConsumer.hpp"

#import "../TaskData.hpp"

void ITaskConsumer::EnqueueTasks(std::vector<std::unique_ptr<TaskData>>&& tasks) {
	enqueuedTasks.enqueue_bulk(std::make_move_iterator(tasks.begin()),
	                           tasks.size());
	nEnqueuedTasks += tasks.size();
}

void ITaskConsumer::EnqueueTasks(std::vector<std::unique_ptr<TaskData>>&& tasks, size_t prodID) {
	enqueuedTasks.enqueue_bulk(requestedTokens[prodID],
	                           std::make_move_iterator(tasks.begin()),
	                           tasks.size());
	nEnqueuedTasks += tasks.size();
}

size_t ITaskConsumer::RequestProducerID() {
	auto i = requestedTokens.size();
	requestedTokens.push_back(moodycamel::ProducerToken(enqueuedTasks));
	return i;
}