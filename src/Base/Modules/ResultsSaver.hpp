//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_RESULTSSAVER_HPP
#define SIMULATOR_RESULTSSAVER_HPP

#include "Base/Interfaces/IResultConsumer.hpp"

#include <atomic>
#include <memory>
#include <thread>

class Settings;
class DataStore;

class ResultsSaver : public IResultConsumer {
 public:
  ResultsSaver(const Settings* settings, std::shared_ptr<DataStore> dataStore);
  virtual ~ResultsSaver();

  void Update();
  void SetConsumer(IResultConsumer* consumer) { _consumer = consumer; };

  inline size_t SavedItems() {
    return savedItems.load(std::memory_order_acquire);
  }

 protected:
  const Settings* _settings;
  std::shared_ptr<DataStore> _dataStore;
  void SaveData(std::unique_ptr<TaskResults> const& results);

  virtual void AllProducersHaveBeenTerminated();

 private:
  void IOThreadUpdate();

  std::vector<std::unique_ptr<TaskResults>> tmpTasksToSave;
  std::thread IOThread;

  IResultConsumer* _consumer = nullptr;

  std::atomic<size_t> savedItems{0};
  std::atomic_bool terminate{false};
};

#endif  // SIMULATOR_RESULTSSAVER_HPP
