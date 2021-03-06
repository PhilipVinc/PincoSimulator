//
// Created by Filippo Vicentini on 23/12/17.
//

#ifndef SIMULATOR_TWMCMANAGER_HPP
#define SIMULATOR_TWMCMANAGER_HPP

#include "Base/Manager.hpp"

#include <memory>
#include <random>
#include <string>
#include <thread>

class DataStore;
class ThreadedTaskProcessor;
class TaskProcessor;
class ResultsSaver;
class TWMCSystemData;
class ProgressReporter;

class TWMCManager : public Manager {
 public:
  explicit TWMCManager(const Settings* settings);
  ~TWMCManager() final;

  void ManagerLoop() final;

 protected:
  void Setup();

 private:
  std::shared_ptr<DataStore> _dataStore;
  std::shared_ptr<ResultsSaver> _saver;
  std::shared_ptr<TaskProcessor> _processor;

  std::shared_ptr<TWMCSystemData> _sysData;
  std::unique_ptr<ProgressReporter> _progressReporter;
  std::string solverName;

  std::mt19937 seedGenerator;

  size_t DispatchTasks();
  std::thread dispatchThread;
};

static ManagerFactory::Registrator<TWMCManager> TWMCBaseSolver =
    ManagerFactory::Registrator<TWMCManager>("TWMC");

// Retrocompatibility fix
static ManagerFactory::Registrator<TWMCManager> TWMCBaseSolverBackup =
    ManagerFactory::Registrator<TWMCManager>("TWMCThread");

#endif  // SIMULATOR_TWMCMANAGER_HPP
