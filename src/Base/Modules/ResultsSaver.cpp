//
// Created by Filippo Vicentini on 22/12/17.
//

#include "ResultsSaver.hpp"

#include "Base/FileFormats/DataStore.hpp"
#include "Base/Settings.hpp"
#include "Base/TaskResults.hpp"

#include <chrono>

#include "easylogging++.h"

ResultsSaver::ResultsSaver(const Settings *settings,
                           std::shared_ptr<DataStore> dataStore) {
  _settings  = settings;
  _dataStore = dataStore;

  while (tmpTasksToSave.size() < 1024) { tmpTasksToSave.emplace_back(nullptr); }

  IOThread = std::thread(&ResultsSaver::IOThreadUpdate, this);
}

ResultsSaver::~ResultsSaver() {
  LOG(INFO) << "Terminating ResultsSaver IOThread.";
  terminate.store(true, std::memory_order_release);
  IOThread.join();
  LOG(INFO) << "Terminated ResultsSaver IOThread and ResultsSaver itself.";
}

void ResultsSaver::Update() {}

void ResultsSaver::IOThreadUpdate() {
  while (!terminate.load(std::memory_order_acquire)) {
    size_t dequeuedTasks =
        enqueuedTasks.try_dequeue_bulk(tmpTasksToSave.begin(), 1024);
    for (size_t i = 0; dequeuedTasks != i; i++) { SaveData(tmpTasksToSave[i]); }
    if (dequeuedTasks == 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      if (_consumer != nullptr) {
        _consumer->EnqueueTasks(std::move(tmpTasksToSave));
      }
    }
  }
}

void ResultsSaver::SaveData(std::unique_ptr<TaskResults> const &results) {
  _dataStore->SaveTaskResults(results);
  savedItems.fetch_add(1, std::memory_order_release);
}

void ResultsSaver::AllProducersHaveBeenTerminated() {
  LOG(INFO) << "ResultsSaver::AllProducersHaveBeenTerminated()";
  terminate.store(true, std::memory_order_release);
}
