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
#include <queue>
#include <vector>

class ThreadedTaskProcessor;
class Solver;
class TaskData;

using namespace std;

class WorkerThread
{
public:
    WorkerThread(size_t id, ThreadedTaskProcessor* manager, Solver* solver);
    ~WorkerThread();
    void WorkerLoop();
	void Terminate() {terminate = true;};
	void TerminateWhenDone() {terminateWhenDone = true;};

	//bool IsFinished();

	float GetSimulationSpeed();
	float GetSimulationProgress();

	void StartProfiling() {	profileEnabled = true; };
	void StopProfiling() {	profileEnabled = false; };

	size_t _id;

protected:
	ThreadedTaskProcessor* _manager;
	Solver* _solver;
	TaskData* _data = nullptr;
	std::vector<TaskData*> _currentTasks;

	bool computing = false;
	bool terminate = false;
	bool terminateWhenDone = false;

	bool profileEnabled = false;
	bool monitoringTime = false;
	chrono::high_resolution_clock::time_point startTime;
	float speed = 0;
};



#endif /* WorkerThread_hpp */
