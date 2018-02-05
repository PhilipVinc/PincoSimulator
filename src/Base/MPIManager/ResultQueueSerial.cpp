//
// Created by Filippo Vicentini on 30/01/18.
//

#include "ResultQueueSerial.hpp"
#include "../Settings.hpp"
#include "../FileFormats/DataStore.hpp"


ResultQueueSerial::ResultQueueSerial(const Settings *settings) {
    _settings = settings;

    tmpTasksToSave = std::vector<TaskResults*>(1024,NULL);
}

void ResultQueueSerial::Update() {
    size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk( tmpTasksToSave.begin(), 1024);
    tmpTasksToSave.resize(dequeuedTasks);
    _consumer->EnqueueTasks(tmpTasksToSave);
    tmpTasksToSave.resize(1024);

    if (dequeuedTasks == 0 && terminate == true) {
        std::cout << "Deleting ResultQueueSerial. " << std::endl;
        delete this;
    }
}

void ResultQueueSerial::SaveData(TaskResults *results) {
    _dataStore->SaveTaskResults(results);
    savedItems++;
}

void ResultQueueSerial::AllProducersHaveBeenTerminated() {
    terminate = true;
}

void ResultQueueSerial::SetConsumer(IResultConsumer* consumer) {
    _consumer = consumer;
}

