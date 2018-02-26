//
// Created by Filippo Vicentini on 23/12/17.
//

#ifndef SIMULATOR_TWMCMANAGER_HPP
#define SIMULATOR_TWMCMANAGER_HPP

#include "Base/Manager.hpp"

#include <random>
#include <string>

class DataStore;
class ThreadedTaskProcessor;
class TaskProcessor;
class ResultsSaver;
class TWMCSystemData;
class TWMCResults;

class TWMCManager : public Manager {

public:
	explicit TWMCManager(const Settings* settings);
	~TWMCManager() final;

    void ManagerLoop() final;
protected:

	void Setup();
private:
	DataStore * _dataStore;
	ResultsSaver* _saver;
	TaskProcessor* _processor;

	TWMCResults* rs;
    TWMCSystemData* _sysData;
    std::string solverName;

    std::mt19937 seedGenerator;

};

static ManagerFactory::Registrator<TWMCManager> TWMCBaseSolver= ManagerFactory::Registrator<TWMCManager>("TWMC");

// Retrocompatibility fix
static ManagerFactory::Registrator<TWMCManager> TWMCBaseSolverBackup= ManagerFactory::Registrator<TWMCManager>("TWMCThread");


#endif //SIMULATOR_TWMCMANAGER_HPP
