//
// Created by Filippo Vicentini on 23/12/17.
//

#include "TWMCManager.hpp"

#include "Base/FileFormats/PincoFormat/PincoFormatDataStore.hpp"
#include "Base/TaskResults.hpp"

#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"

#include "Base/Modules/ProgressReporter.hpp"
#include "Base/Modules/ResultsSaver.hpp"
#include "Base/NoisyMatrix.hpp"
#include "Base/TaskProcessors/ThreadedTaskProcessor/ThreadedTaskProcessor.hpp"
#include "Base/TaskResults.hpp"

#include "Base/Exceptions/NoEligibleSolverException.hpp"

#include "easylogging++.h"

#include <chrono>
#include <limits>
#include <memory>
#include <vector>

#ifdef MPI_SUPPORT
#include "Base/TaskProcessors/MPITaskProcessor/MPIProcessor.hpp"
#endif

template <typename T>
static bool AreEqual(T f1, T f2) {
  return (std::fabs(f1 - f2) <=
          std::numeric_limits<T>::epsilon() * std::fmax(fabs(f1), fabs(f2)));
}

TWMCManager::TWMCManager(const Settings *settings) : Manager(settings) {
  // Choose the solver
  _sysData = std::make_shared<TWMCSystemData>(settings);

  // TODO: Retrocompatibility fix
  // If we are running old simulations, then PBC is sottointesa.
  if (settings->get<string>("Manager") == "TWMCThread") {
    _sysData->PBC = true;
  }
  if (_sysData->latticeName == "lieb")
    solverName = "TWMCLieb";
  else if (_sysData->PBC) {
    if (!_sysData->F->HasTimeDependence()) {
      solverName = "TWMCBase";
    } else {
      solverName = "TWMCThermo";
    }
  } else {
    LOG(ERROR) << "ERROR: No solver available.";
    throw NoEligibleSolver();
  }

  LOG(INFO) << "Using Solver: " << solverName;
}

TWMCManager::~TWMCManager() {
  // dispatchThread.join();

  delete _saver;
  delete _dataStore;
}

#ifdef MPI_SUPPORT
#include "TWMCResults.hpp"
#endif

void TWMCManager::Setup() {
  seedGenerator = mt19937(settings->GlobalSeed());

  _dataStore = new PincoFormatDataStore(settings, settings->GetOutputFolder());

  _saver = new ResultsSaver(settings, _dataStore);

#ifdef MPI_SUPPORT
  int ppnMPI = settings->get<int>("ppn", -1);
  if (ppnMPI == -1) { ppnMPI = settings->get<int>("processes", -1); }

  if (settings->mpiWorldSize > 1) {
    std::shared_ptr<MPIProcessor> mpiManager =
        std::make_shared<MPIProcessor>(solverName, ppnMPI, ppnMPI);
    mpiManager->ProvideMPICommunicator(nullptr);
    _processor = mpiManager;
  } else {
    _processor =
        std::make_shared<ThreadedTaskProcessor>(solverName, ppnMPI, ppnMPI);
  }

#else
  _processor = std::make_shared<ThreadedTaskProcessor>(
      solverName, settings->get<int>("processes", -1),
      settings->get<int>("max_processes", -1));
#endif

  _processor->SetConsumer(_saver);
  _processor->Setup();
  _progressReporter = std::make_unique<ProgressReporter>(settings, _processor);

  // Print time and other stuff
  auto times = _sysData->GetStoredTimes();
  _dataStore->SaveFile("_t.dat", times);
  if (_sysData->F->HasTimeDependence()) {
    auto F_t = _sysData->GetStoredVariableEvolution(_sysData->F);
    _dataStore->SaveFile("_F_t.dat", F_t);
  }
}

void TWMCManager::ManagerLoop() {
  // Recover ids alredy used
  std::set<size_t> usedIds = _dataStore->UsedIds();
  // dispatchThread = std::thread(&TWMCManager::DispatchTasks, this);

  size_t nTaskToEnqueue = DispatchTasks();
  _progressReporter->SetNOfTasks(nTaskToEnqueue);

  cout << "Enqueued " << nTaskToEnqueue << " tasks. " << endl;

  while (_saver->savedItems < nTaskToEnqueue) {
    _processor->Update();
    _saver->Update();
    _progressReporter->Update();
    // std::this_thread::sleep_for(chrono::milliseconds(100));
  }

  cout << "Exited the ManagerLoop()" << endl;
}

size_t TWMCManager::DispatchTasks() {
  std::set<size_t> usedIds = _dataStore->UsedIds();
  size_t nTaskToEnqueue    = settings->get<size_t>("n_traj");
  size_t nEnqueuedTasks    = 0;
  double tEnd              = settings->get<size_t>("t_end");

  LOG(INFO) << "# of used Ids: " << usedIds.size();

  // Add trajectories to reach the end point.
  if (usedIds.size() == 0 || usedIds.size() < nTaskToEnqueue) {
    // Find the maxId used, so that following trajectories will be over that
    // value
    size_t maxId = 0;
    for (size_t el : usedIds) {
      if (el > maxId) { maxId = el; }
    }
    nTaskToEnqueue -= usedIds.size();
    // Enqueue tasks to the end.
    {
      std::vector<std::unique_ptr<TaskData>> tasks;
      tasks.reserve(nTaskToEnqueue);
      for (size_t i = 0; i < nTaskToEnqueue; i++) {
        maxId++;

        TWMCTaskData *tmp = new TWMCTaskData();
        tmp->systemData   = _sysData;
        tmp->t_start      = _sysData->t_start;
        tmp->t_end        = _sysData->t_end;
        tmp->initialCondition =
            TWMCTaskData::InitialConditions::ReadFromSettings;
        tmp->rngSeed           = seedGenerator();
        tmp->storeInitialState = true;
        tmp->id                = maxId;

        std::unique_ptr<TaskData> tmp2(tmp);
        tasks.push_back(std::move(tmp2));
      }
      nEnqueuedTasks += tasks.size();
      _processor->EnqueueTasks(std::move(tasks));
    }
  }

  // Check that the endTime is right.
  if (usedIds.size() != 0) {
    std::vector<std::unique_ptr<TaskData>> tasks;
    for (size_t id : usedIds) {
      std::unique_ptr<TaskResults> old = (_dataStore->LoadEndFrame(id));

      std::unique_ptr<TWMCResults> oldRes =
          unique_ptr<TWMCResults>{static_cast<TWMCResults *>(old.release())};
      if ((oldRes->extraDataMemory[1] + _sysData->dt_obs) < tEnd) {
        TWMCTaskData *tmp = new TWMCTaskData();
        tmp->systemData   = _sysData;
        tmp->t_start      = oldRes->extraDataMemory[1];
        tmp->t_end        = tEnd;
        tmp->initialCondition =
            TWMCTaskData::InitialConditions::ReadFromPreviousData;
        tmp->rngSeed           = seedGenerator();
        tmp->id                = oldRes->GetId();
        tmp->storeInitialState = false;
        tmp->prevData          = std::move(oldRes);
        std::unique_ptr<TaskData> tmp2(tmp);
        tasks.push_back(std::move(tmp2));
      }
    }
    if (tasks.size() != 0) {
      nEnqueuedTasks += tasks.size();
      _processor->EnqueueTasks(std::move(tasks));
    }
  }
  return nEnqueuedTasks;
}