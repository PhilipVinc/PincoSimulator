//
// Created by Filippo Vicentini on 22/12/17.
//

#include "ResultsSaver.hpp"
#include "Settings.hpp"
#include "FileFormats/DataStore.hpp"


ResultsSaver::ResultsSaver(const Settings *settings, DataStore *dataStore) {
	_settings = settings;
	_dataStore = dataStore;

	tmpTasksToSave = std::vector<TaskResults*>(1024,NULL);
}

void ResultsSaver::Update() {
	size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk( tmpTasksToSave.begin(), 1024);
	for (size_t i = 0; dequeuedTasks != i; i++)
	{
		SaveData(tmpTasksToSave[i]);
		delete tmpTasksToSave[i];
	}

	if (dequeuedTasks == 0 && terminate == true) {
		std::cout << "Deleting ResultSaver. " << std::endl;
		delete this;
	}
}

void ResultsSaver::SaveData(TaskResults *results)
{
	_dataStore->SaveTaskResults(results);
	savedItems++;
}

void ResultsSaver::AllProducersHaveBeenTerminated()
{
	terminate = true;
}
