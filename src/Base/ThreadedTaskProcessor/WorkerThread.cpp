//
//  WorkerThread.cpp
//  DataElaborator
//
//  Created by Filippo Vicentini on 24/03/14.
//  Copyright (c) 2014 Filippo Vicentini. All rights reserved.
//

#include "WorkerThread.hpp"

#include "ThreadedTaskProcessor.hpp"
#include "Base/Solver.hpp"
#include "Base/TaskData.hpp"
#include "../TaskResults.hpp"
#include <iostream>


WorkerThread::WorkerThread(size_t id, ThreadedTaskProcessor* manager, Solver* solver)
{
	_id = id;
	_manager = manager;
	_solver = solver;

	computing = false;
	terminate = false;

	/*while(_currentTasks.size() < _solver->nTasksToRequest) {
		_currentTasks.emplace_back(std::unique_ptr<TaskData>(nullptr));
	}*/

}

WorkerThread::~WorkerThread()
{
    delete _solver;
	if (_data != nullptr)
		delete _data;
	/*std::for_each(_currentTasks.begin(),
	              _currentTasks.end(),
	              std::default_delete<TaskData>());*/
}

void WorkerThread::WorkerLoop()
{
    while (!terminate)
    {
	    _currentTasks = _manager->GetDispatchedTasks(_id, _solver->nTasksToRequest);
	    if (_currentTasks.size() != 0)
        {
            // Profiler
            if (profileEnabled)
            {
                startTime = std::chrono::high_resolution_clock::now();
                monitoringTime = true;
            }

	        computing = true;
            std::vector<unique_ptr<TaskResults>> results = _solver->Compute(_currentTasks);
            _manager->GiveResults(_id, std::move(results));
	        computing = false;

            if (profileEnabled)
            {
                auto t = chrono::high_resolution_clock::now();
                std::chrono::duration<float> dT = t - startTime;
                float dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dT).count();
                speed = 1.0/dt_ms;
                monitoringTime = false;
                _manager->ReportAverageSpeed(speed);
            }
        } else if (terminateWhenDone == true) {
	        terminate = true;
        }
    }
    _manager->ReportThreadTermination(_id);
}

// Optimizer
float WorkerThread::GetSimulationSpeed()
{
	if (_currentTasks.size() != 0 && monitoringTime)
	{
		auto t = chrono::high_resolution_clock::now();
		std::chrono::duration<float> dT = t - startTime;
		float dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dT).count();

		return _solver->ApproximateComputationProgress()/dt_ms;
	}
	else
	{
		return speed;
	}
	return 0;
}

float WorkerThread::GetSimulationProgress()
{
	if (computing) {
		return _solver->ApproximateComputationProgress();
	}
	return 0;
}

