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
}

void ResortManager::Setup() {
  LOG(INFO) << "Loading data from: "
            << settings->get<std::string>("oldArchive");
  std::shared_ptr<PincoFormatDataStore> ds_old =
          std::make_shared<PincoFormatDataStore>(settings,
                                                 settings->get<std::string>("oldArchive"));
  _dataStore_old = std::static_pointer_cast<DataStore>(ds_old);

  LOG(INFO) << "Saving data to: " << settings->GetOutputFolder();
  std::shared_ptr<PincoFormatDataStore> ds_new =
          std::make_shared<PincoFormatDataStore>(settings,
                                                 settings->GetOutputFolder());
  _dataStore_new = std::static_pointer_cast<DataStore>(ds_new);

  LOG(INFO) << "succesfully created archives";
  _saver = std::make_unique<ResultsSaver>(settings, _dataStore_new);
}

void ResortManager::ManagerLoop() {
  std::set<size_t> usedIds = _dataStore_old->UsedIds();
  size_t nEnqueuedTasks    = 0;
  LOG(INFO) << "# of used Ids: " << usedIds.size();

  for (size_t id : usedIds) {
    size_t savedItems = _saver->SavedItems();
    std::cout << "Loaded : " << nEnqueuedTasks << "/" << usedIds.size()
              << " - attempting # " << id << std::endl;
    std::cout << " Saved : " << savedItems << "/" << usedIds.size()
              << " - in queue : " << nEnqueuedTasks - savedItems
              << std::endl;
    std::cout << "\x1b[A\r" << std::flush;

    // LOG(INFO) << "Loading #" << id;
    std::vector<std::unique_ptr<TaskResults>> oldTasks;
    nEnqueuedTasks++;
    oldTasks.push_back(std::move(_dataStore_old->LoadTaskResults(id)));
    // LOG(INFO) << "Loaded #" << id;
    _saver->EnqueueTasks(std::move(oldTasks));
  }

  while (_saver->SavedItems() < nEnqueuedTasks) {
    _saver->Update();
    LOG(INFO) << "Saved " << _saver->SavedItems() << " items.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
