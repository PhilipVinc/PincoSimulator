//
// Created by Filippo Vicentini on 22/12/17.
//

#include "ResultsSaver.hpp"
#include "Settings.hpp"
#include "FileFormats/DataStore.hpp"

#include "TaskResults.hpp"

#include <chrono>

ResultsSaver::ResultsSaver(const Settings *settings, DataStore *dataStore) {
	_settings = settings;
	_dataStore = dataStore;

	while (tmpTasksToSave.size() < 1024) {
		tmpTasksToSave.emplace_back(nullptr);
	}

    IOThread = std::thread(&ResultsSaver::IOThreadUpdate, this);
}

ResultsSaver::~ResultsSaver() {
    terminate = true;
    IOThread.join();
}

void ResultsSaver::Update() {

}

void ResultsSaver::IOThreadUpdate() {
    while (!terminate) {
        size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk(tmpTasksToSave.begin(), 1024);
        for (size_t i = 0; dequeuedTasks != i; i++) {
            SaveData(tmpTasksToSave[i]);
        }
        if (dequeuedTasks == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else {
            if (_consumer != nullptr) {
                _consumer->EnqueueTasks(std::move(tmpTasksToSave));
            }
        }
    }
}

void ResultsSaver::SaveData(std::unique_ptr<TaskResults> const &results)
{
	_dataStore->SaveTaskResults(results);
	savedItems++;
}

void ResultsSaver::AllProducersHaveBeenTerminated()
{
	terminate = true;
}
