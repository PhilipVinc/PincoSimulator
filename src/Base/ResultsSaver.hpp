//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_RESULTSSAVER_HPP
#define SIMULATOR_RESULTSSAVER_HPP

#include "Interfaces/IResultConsumer.hpp"

class Settings;
class DataStore;

class ResultsSaver : public IResultConsumer
{
public:
	ResultsSaver(const Settings* settings, DataStore* dataStore);
	virtual ~ResultsSaver() {};

	void Update();

	size_t savedItems = 0;
protected:
	const Settings* _settings;
	DataStore* _dataStore;
	void SaveData(std::unique_ptr<TaskResults> const &results);


	virtual void AllProducersHaveBeenTerminated();
private:
	std::vector<std::unique_ptr<TaskResults>> tmpTasksToSave;

	bool terminate = false;

};


#endif //SIMULATOR_RESULTSSAVER_HPP
