//
// Created by Filippo Vicentini on 22/12/17.
//

#include "IResultConsumer.hpp"

#include "../TaskResults.hpp"

#include "easylogging++.h"

void IResultConsumer::EnqueueTasks(
    std::vector<std::unique_ptr<TaskResults>>&& tasks) {
  enqueuedTasks.enqueue_bulk(std::make_move_iterator(tasks.begin()),
                             tasks.size());
  nEnqueuedTasks += tasks.size();
}

void IResultConsumer::EnqueueTasks(
    std::vector<std::unique_ptr<TaskResults>>&& tasks, size_t prodID) {
  enqueuedTasks.enqueue_bulk(requestedTokens[prodID],
                             std::make_move_iterator(tasks.begin()),
                             tasks.size());
  nEnqueuedTasks += tasks.size();
}

size_t IResultConsumer::RequestProducerID() {
  auto i = requestedTokens.size();
  requestedTokens.push_back(moodycamel::ProducerToken(enqueuedTasks));
  return i;
}

void IResultConsumer::ReportProducerTermination(std::vector<size_t> IDs) {
  LOG(DEBUG) << "IResultConsumer::ReportProducerTermination()";
  for (size_t i = 0; i < IDs.size(); i++) { disabledTokens.push_back(IDs[i]); }

  if (disabledTokens.size() == requestedTokens.size()) {
    LOG(INFO) << "AllProducerHaveBeenTerminated Terminated.";
    AllProducersHaveBeenTerminated();
  }
  LOG(DEBUG) << "CompletedReport.";
}