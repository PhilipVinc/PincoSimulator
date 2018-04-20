//
// Created by Filippo Vicentini on 20/12/2017.
//

#ifndef SIMULATOR_ITASKCONSUMER_HPP
#define SIMULATOR_ITASKCONSUMER_HPP

#include "Libraries/concurrentqueue.h"

#include <memory>
#include <vector>
class TaskData;

class ITaskConsumer {
 public:
  void EnqueueTasks(std::vector<std::unique_ptr<TaskData>>&& tasks);
  void EnqueueTasks(std::vector<std::unique_ptr<TaskData>>&& tasks,
                    size_t prodID);

  size_t RequestProducerID();

 protected:
  // Queue of tasks that are waiting to be executed by a worker.
  // Fed by Manager, Eaten by Workers
  moodycamel::ConcurrentQueue<std::unique_ptr<TaskData>> enqueuedTasks;
  size_t nEnqueuedTasks = 0;

  // Producers registered with a token. The producer ID is the index
  // used to access the token vector.
  std::vector<moodycamel::ProducerToken> requestedTokens;

 private:
};

#endif  // SIMULATOR_ITASKCONSUMER_HPP
