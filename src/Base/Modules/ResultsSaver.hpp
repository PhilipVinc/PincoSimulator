//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_RESULTSSAVER_HPP
#define SIMULATOR_RESULTSSAVER_HPP

#include "Base/Interfaces/IResultConsumer.hpp"

#include <thread>

class Settings;
class DataStore;

class ResultsSaver : public IResultConsumer
{
public:
	ResultsSaver(const Settings* settings, DataStore* dataStore);
	virtual ~ResultsSaver();

	void Update();
	void SetConsumer(IResultConsumer* consumer) { _consumer = consumer;};

	size_t savedItems = 0;
protected:
	const Settings* _settings;
	DataStore* _dataStore;
	void SaveData(std::unique_ptr<TaskResults> const &results);


	virtual void AllProducersHaveBeenTerminated();
private:
    void IOThreadUpdate();

    std::vector<std::unique_ptr<TaskResults>> tmpTasksToSave;
	std::thread IOThread;

	bool terminate = false;
	IResultConsumer* _consumer = nullptr;

};


#endif //SIMULATOR_RESULTSSAVER_HPP
