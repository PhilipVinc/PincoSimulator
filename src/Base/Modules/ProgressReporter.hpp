//
// Created by Filippo Vicentini on 20/04/2018.
//

#ifndef SIMULATOR_PROGRESSREPORTER_HPP
#define SIMULATOR_PROGRESSREPORTER_HPP

#include "Base/Interfaces/IResultConsumer.hpp"

#include <atomic>
#include <memory>
#include <thread>

class Settings;
class TaskProcessor;

class ProgressReporter {
 public:
  ProgressReporter(const Settings* settings,
                   const std::shared_ptr<TaskProcessor> processor,
                   bool forceFile = false);
  virtual ~ProgressReporter();

  void Update();

  void SetNOfTasks(size_t _nTasks) { nTasks = _nTasks; };

 private:
  void IOThreadUpdate();
  const std::shared_ptr<TaskProcessor> processor;

  std::thread IOThread;
  enum logType { file, dumb, smart };
  logType logging;

  size_t nTasks = 0;

  bool started   = false;
  std::atomic<bool> terminate;
};

#endif  // SIMULATOR_PROGRESSREPORTER_HPP
