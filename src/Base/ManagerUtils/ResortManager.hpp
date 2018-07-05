//
// Created by Filippo Vicentini on 24/04/2018.
//

#ifndef SIMULATOR_RESORTMANAGER_HPP
#define SIMULATOR_RESORTMANAGER_HPP

#include "Base/Manager.hpp"

#include <memory>
#include <random>
#include <string>
#include <thread>

class DataStore;
class ResultsSaver;
class ProgressReporter;

class ResortManager : public Manager {
 public:
  explicit ResortManager(const Settings* settings);
  ~ResortManager() final;

  void ManagerLoop() final;

 protected:
  void Setup();

 private:
  std::shared_ptr<DataStore> _dataStore_old;
  std::shared_ptr<DataStore> _dataStore_new;
  std::unique_ptr<ResultsSaver> _saver;

  std::unique_ptr<ProgressReporter> _progressReporter;
  std::string solverName;
};

static ManagerFactory::Registrator<ResortManager> ResortManagerReg =
    ManagerFactory::Registrator<ResortManager>("reblock");

#endif  // SIMULATOR_RESORTMANAGER_HPP
