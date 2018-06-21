//
// Created by Filippo Vicentini on 28/05/2018.
//

#ifndef SIMULATOR_MERGEMANAGER_HPP
#define SIMULATOR_MERGEMANAGER_HPP

#include "Base/Manager.hpp"

#include <memory>
#include <random>
#include <string>
#include <thread>

class DataStore;
class ResultsSaver;
class ProgressReporter;

class MergeManager : public Manager{
 public:
  explicit MergeManager(const Settings* settings);
  ~MergeManager() final;

  void ManagerLoop() final;

 protected:
  void Setup();

 private:
  DataStore* _dataStore_old1;
  DataStore* _dataStore_old2;
  DataStore* _dataStore_new;
  std::unique_ptr<ResultsSaver> _saver;

  std::unique_ptr<ProgressReporter> _progressReporter;
  std::string solverName;
};

static ManagerFactory::Registrator<MergeManager> ResortManagerReg =
    ManagerFactory::Registrator<MergeManager>("merge");

#endif  // SIMULATOR_MERGEMANAGER_HPP
