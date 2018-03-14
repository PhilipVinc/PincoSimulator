//
// Created by Filippo Vicentini on 20/12/2017.
//

#ifndef SIMULATOR_THREADEDTASKPROCESSOR_HPP
#define SIMULATOR_THREADEDTASKPROCESSOR_HPP

#import "Base/TaskProcessor.hpp"
#include "Libraries/concurrentqueue.h"

#include <chrono>
#include <stdio.h>
#include <queue>
#include <vector>
#include <thread>

class Settings;
class WorkerThread;


class ThreadedTaskProcessor : public TaskProcessor
{
public:
	ThreadedTaskProcessor(std::string solverName, int _processes, int _max_processes = 0);
    virtual ~ThreadedTaskProcessor();

    virtual void Setup() final;
	virtual void Update() final;


	// -------- Called From Workers ---------- //
	// Take a task from dispatchedTasks to execute it (called by a worker)
	std::vector<std::unique_ptr<TaskData>> GetDispatchedTasks(size_t th_id, size_t maxTasks =1);
	void GiveCompletedResults(size_t th_id, std::vector<TaskResults*> res);
	void GiveCompletedResults(size_t th_id, TaskResults* res);
	// -------------------------------------- //

    // Return a completed task, to add it to elaboratedTasks
    void GiveResults(size_t th_id, std::unique_ptr<TaskResults> task);
    void GiveResults(size_t th_id, std::vector<std::unique_ptr<TaskResults>>&& tasks);
    // Method called by a thread to inform the manager that he is now dead
    // and can be joined.
    void ReportThreadTermination(size_t th_id);
	virtual void AllProducersHaveBeenTerminated() final;

    void Terminate();
	void TerminateWhenDone();

protected:
	// Redef
	size_t nTasksEnqueued = 0;

	// Threading support
	size_t nThreads = 1;
#ifdef GPU_SUPPORT
	size_t nGPUThreads = 0;
	size_t nAvailableGPUs = 0;
#endif

	// --- old---
    size_t nTasksLeftToEnqueue = 0;
    size_t nTasksToSave = 0;
    size_t nTasksFillCapacity = 1;
    size_t nCompletedTasks = 0;
    size_t nTotalTasks = 0;

private:
    size_t nextWorkerId = 0;
    std::vector<size_t> activeThreads;
    std::queue<size_t> unactivatedThreadPool;
    moodycamel::ConcurrentQueue<size_t> threadsToJoin;


    std::vector<std::thread> threads;
    std::vector<WorkerThread*> workers;
    std::vector<size_t> workerProducerID;
	std::vector<size_t> workerCompletedTasks;

	bool terminate = false;
	bool terminateWhenDone = false;

    void CreateWorker(Solver* solver);
	void TerminateWorker(size_t i);
	void TerminateWorkerWhenDone(size_t i);
	void TerminateAllWorkers();
	void TerminateAllWorkersWhenDone();
    void JoinThread(size_t th_id);

    // Optimizer Stuff
    void EnableProfilingInWorkers();
    void DisableProfilingInWorkers();
    void ComputeAverageSpeed();
    bool profiling = true;
    size_t maxProcesses = 0;
    float averageTaskComputationTime = 0.0;
    int sleepTimeMs = 1000;

	std::chrono::system_clock::time_point lastOptTime;
	std::chrono::system_clock::time_point lastPrintTime;
	std::chrono::system_clock::time_point startTime;
	std::chrono::system_clock::duration deltaTOpt;
	std::chrono::system_clock::duration deltaTPrint;

    // Printing stuff
    size_t lastMsgLength = 0;

public:
	void ReportAverageSpeed(float speed);
	virtual size_t NumberOfCompletedTasks() final;
	virtual float Progress() final;

};


#endif //SIMULATOR_THREADEDTASKPROCESSOR_HPP
