//
//  WorkerThread.hpp
//  DataElaborator
//
//  Created by Filippo Vicentini on 24/03/14.
//  Copyright (c) 2014 Filippo Vicentini. All rights reserved.
//

#ifndef WorkerThread_hpp
#define WorkerThread_hpp

#include <atomic>
#include <chrono>
#include <stdio.h>
#include <memory>
#include <queue>
#include <vector>

class ThreadedTaskProcessor;
class Solver;
class TaskData;

using namespace std;

/*! 
    @brief Controls a thread that will check a given queue for TaskData objects to process through Solver _solver.
 
    @discussion This class should be created by a ThreadedTaskManager or similar objects.
 */
class WorkerThread
{
public:
    WorkerThread(size_t id, ThreadedTaskProcessor* manager, Solver* solver);
    ~WorkerThread();
    void WorkerLoop();
	void Terminate() {terminate.store(true, std::memory_order_release);};
	void TerminateWhenDone() {terminateWhenDone.store(true, std::memory_order_release);};

	float GetSimulationSpeed();
	float GetSimulationProgress();

	void StartProfiling() {	profileEnabled = true; };
	void StopProfiling() {	profileEnabled = false; };

	size_t _id;

protected:
	ThreadedTaskProcessor* _manager;
	Solver* _solver;
	std::vector<std::unique_ptr<TaskData>> _currentTasks;

	bool computing = false;
  std::atomic_bool terminate{false};
  std::atomic_bool terminateWhenDone{false};

	bool profileEnabled = false;
	bool monitoringTime = false;
	chrono::high_resolution_clock::time_point startTime;
	float speed = 0;
	size_t completedTasks = 0;
};



#endif /* WorkerThread_hpp */
