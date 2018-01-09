//
// Created by Filippo Vicentini on 23/12/17.
//

#include "TWMCManager.hpp"

#include "Base/FileFormats/PincoFormatDataStore.hpp"

#include "TWMC/TWMCSystemData.hpp"
#include "TWMC/TWMCTaskData.hpp"

#include "ThreadedManager/ThreadedTaskProcessor.hpp"
#include "Base/ResultsSaver.hpp"
#include "Base/NoisyMatrix.hpp"
#include "Base/Utils/StringFormatter.hpp"

#include <vector>
#include <chrono>

TWMCManager::TWMCManager(const Settings *settings) : Manager(settings) {
	Setup();
}

TWMCManager::~TWMCManager() {
	delete _processor;
	delete _saver;
	delete _dataStore;
}

void TWMCManager::Setup() {
	// Choose the solver
    _sysData = new TWMCSystemData(settings);
    if (_sysData->latticeName == "lieb")
        solverName = "TWMCLieb";
    else if (_sysData->PBC ) {
        if (! _sysData->F->HasTimeDependence()) {
            solverName = "TWMCBase";
        } else {
            solverName = "TWMCThermo";
        }
    } else {
        std::cerr << "ERROR: No solver available." << std::endl;
        return;
    }

    std::cout << "Using Solver: " << solverName << std::endl;
    seedGenerator = mt19937(settings->GlobalSeed());

    _dataStore = new PincoFormatDataStore(settings, settings->GetRootFolder());

	_saver = new ResultsSaver(settings, _dataStore);
	_processor = new ThreadedTaskProcessor(settings, solverName);

	_processor->SetConsumer(_saver);
	_processor->Setup();

    // Print time and other stuff
    auto times = _sysData->GetStoredTimes();
    _dataStore->SaveFile("_t.dat", times);
    if (_sysData->F->HasTimeDependence()) {
        auto F_t = _sysData->GetStoredVariableEvolution(_sysData->F);
        _dataStore->SaveFile("_F_t.dat", F_t);
    }

}

void TWMCManager::ManagerLoop() {
    size_t nTaskToEnqueue = settings->get<size_t>("n_traj");
	std::vector<TaskData*> tasks(nTaskToEnqueue, nullptr);
	for (size_t i = 0; i < nTaskToEnqueue; i++)
	{
		auto tmp = new TWMCTaskData();
		tmp->systemData = _sysData;
		tmp->t_start = _sysData->t_start;
		tmp->t_end = _sysData->t_end;
		tmp->initialCondition = TWMCTaskData::InitialConditions ::ReadFromSettings;
		tmp->rngSeed = seedGenerator();
		tasks[i] = tmp;
	}

    // Time
    chrono::system_clock::time_point startTime = chrono::system_clock::now();
    chrono::system_clock::time_point lastPrintTime = startTime;
    chrono::system_clock::duration deltaTPrint = chrono::seconds(1);
    size_t lastMsgLength = 0;
    // -----end time

    _processor->EnqueueTasks(tasks);
	while(_saver->savedItems < nTaskToEnqueue)
	{
		_processor->Update();
		_saver->Update();

        auto now = chrono::system_clock::now();
        chrono::system_clock::duration dt = now-lastPrintTime;
        if ( dt > deltaTPrint) {
            chrono::system_clock::duration elapsed = now-startTime;
            int deltaTc = chrono::duration_cast<chrono::seconds>(elapsed).count();

            auto nCompletedTasks = _processor->NumberOfCompletedTasks();
            auto progress = _processor->Progress() / float(nTaskToEnqueue)*100;

            std::string tmptmp = progress!= 0 ? std::to_string(int(deltaTc/progress*100)):"***";
            std::string msgString = string_format("(%.2f%%) Completed %i/%i.   Time: (%i/%s). ",  progress,
                                                  nCompletedTasks,
                                                  nTaskToEnqueue, deltaTc,
                                                  tmptmp.c_str());

            std::string tmp = std::string(lastMsgLength, '\b');
            lastMsgLength = msgString.length() -1;
            msgString = tmp + msgString;
            cout << msgString;
            fflush(stdout);

            lastPrintTime = now;
        }
	}
	_processor->AllProducersHaveBeenTerminated();


}