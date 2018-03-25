//
// Created by Filippo Vicentini on 22/12/17.
//

#include "ResultsSaver.hpp"
#include "Settings.hpp"
#include "FileFormats/DataStore.hpp"

#include "TaskResults.hpp"



ResultsSaver::ResultsSaver(const Settings *settings, DataStore *dataStore) {
	_settings = settings;
	_dataStore = dataStore;

	while (tmpTasksToSave.size() < 1024) {
		tmpTasksToSave.emplace_back(nullptr);
	}
}

void ResultsSaver::Update() {
	size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk( tmpTasksToSave.begin(), 1024);
	for (size_t i = 0; dequeuedTasks != i; i++)
	{
		SaveData(tmpTasksToSave[i]);


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
