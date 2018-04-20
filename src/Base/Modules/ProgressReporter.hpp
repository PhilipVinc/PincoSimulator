//
// Created by Filippo Vicentini on 20/04/2018.
//

#ifndef SIMULATOR_PROGRESSREPORTER_HPP
#define SIMULATOR_PROGRESSREPORTER_HPP

#include "Base/Interfaces/IResultConsumer.hpp"

#include <thread>

class Settings;
class TaskProcessor;

class ProgressReporter {
 public:
  ProgressReporter(const Settings* settings, const TaskProcessor* processor,
                   bool forceFile = false);
  virtual ~ProgressReporter();

  void Update();

  void SetNOfTasks(size_t _nTasks) { nTasks = _nTasks; };

 private:
  void IOThreadUpdate();
  const TaskProcessor* processor;

  std::thread IOThread;
  enum logType { file, dumb, smart };
  logType logging;

  size_t nTasks = 0;

  bool started               = false;
  bool terminate             = false;
};

#endif  // SIMULATOR_PROGRESSREPORTER_HPP
