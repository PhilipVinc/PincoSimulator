//
// Created by Filippo Vicentini on 24/04/2018.
//

#include "ResortManager.hpp"

#include "Base/FileFormats/PincoFormat/PincoFormatDataStore.hpp"
#include "Base/TaskResults.hpp"

#include "Base/Modules/ProgressReporter.hpp"
#include "Base/Modules/ResultsSaver.hpp"
#include "Base/TaskResults.hpp"

#include "easylogging++.h"

#include <chrono>
#include <limits>
#include <memory>
#include <vector>

ResortManager::ResortManager(const Settings *settings) : Manager(settings) {
  LOG(INFO) << "Creating Resort Manager. ";
}

ResortManager::~ResortManager() {
  delete _dataStore_old;
  delete _dataStore_new;
}

void ResortManager::Setup() {
  LOG(INFO) << "Loading data from: "
            << settings->get<std::string>("oldArchive");
  _dataStore_old = new PincoFormatDataStore(
      settings, settings->get<std::string>("oldArchive"));
  LOG(INFO) << "Saving data to: " << settings->GetOutputFolder();
  _dataStore_new =
      new PincoFormatDataStore(settings, settings->GetOutputFolder());
  LOG(INFO) << "succesfully created archives";

  LOG(INFO) << "succesfully created archives";
  _saver = std::make_unique<ResultsSaver>(settings, _dataStore_new);
}

void ResortManager::ManagerLoop() {
  std::set<size_t> usedIds = _dataStore_old->UsedIds();
  size_t nEnqueuedTasks    = 0;
  LOG(INFO) << "# of used Ids: " << usedIds.size();

  for (size_t id : usedIds) {
    std::cout << "Loaded : " << nEnqueuedTasks << "/" << usedIds.size()
              << " - attempting # " << id << std::endl;
    std::cout << " Saved : " << _saver->savedItems << "/" << usedIds.size()
              << " - in queue : " << nEnqueuedTasks - _saver->savedItems
              << std::endl;
    std::cout << "\x1b[A\r" << std::flush;

    // LOG(INFO) << "Loading #" << id;
    std::vector<std::unique_ptr<TaskResults>> oldTasks;
    nEnqueuedTasks++;
    oldTasks.push_back(std::move(_dataStore_old->LoadTaskResults(id)));
    // LOG(INFO) << "Loaded #" << id;
    _saver->EnqueueTasks(std::move(oldTasks));
  }

  while (_saver->savedItems < nEnqueuedTasks) {
    _saver->Update();
    LOG(INFO) << "Saved " << _saver->savedItems << " items.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
