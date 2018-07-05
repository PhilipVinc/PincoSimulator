//
// Created by Filippo Vicentini on 20/12/2017.
//

#include "ThreadedTaskProcessor.hpp"

#include "Base/Solver.hpp"
#include "Base/TaskData.hpp"
#include "WorkerThread.hpp"

#include "easylogging++.h"


#ifdef GPU_SUPPORT
#include <vexcl/vexcl.hpp>
#include "Base/SolverGPU.hpp"
#endif

#include <numeric>


ThreadedTaskProcessor::ThreadedTaskProcessor(std::string solverName, int _processes, int _max_processes) :
		TaskProcessor(solverName)
{
	size_t n = std::thread::hardware_concurrency();
	size_t nset = 0;
	int ntmp = _processes;
	if (ntmp == -1)
		nset = 1000;
	else
		nset = ntmp;


#ifdef GPU_SUPPORT
	nThreads = std::min(n, nset);
#else
    nset = nset == 0? 20 : nset;
    nThreads = (n == 0 ? 20 : _processes );
#endif
	maxProcesses = _max_processes;
	if (maxProcesses <= 0 || maxProcesses > 1000)
		maxProcesses = 2*n;
	nThreads = min(nThreads, maxProcesses);
}

ThreadedTaskProcessor::~ThreadedTaskProcessor()
{
	LOG(INFO) << "Starting deletion of ThreadedTaskProcessor's Workers";
	TerminateAllWorkers();
	while(activeThreads.size()!= 0)
	{
		size_t th_id;
		while (threadsToJoin.try_dequeue(th_id))
		{
			LOG(INFO) << ("Deactivating thread #" + to_string(th_id));
			JoinThread(th_id);
    }
	}
	LOG(INFO) << "Destroyed ThreadedTaskProcessor";
}

void ThreadedTaskProcessor::AllProducersHaveBeenTerminated() {
	this->TerminateWhenDone();
}

void ThreadedTaskProcessor::Setup()
{
  LOG(INFO) << "Initializing " << nThreads << " threads.";
	for (size_t i = 0; i != nThreads; i++)
	{
		Solver* solver = SolverFactory::makeRawNewInstance(_solverName);
		if (solver != nullptr)
			CreateWorker(solver);
	}
	nTasksFillCapacity = nThreads*3;

#ifdef GPU_SUPPORT
    vex::Context ctx( vex::Filter::GPU && vex::Filter::Env );
    int nAvailableGPUs = ctx.size();
	int nMaxGPUs = _settings->get<int>("GPUs", -1);
	if (nMaxGPUs  < 0)
		nMaxGPUs = 100000;
	nAvailableGPUs = std::min(nAvailableGPUs, nMaxGPUs);

	size_t worksizeGPU = 0;
	std::string gpuSolver = _solverName+"_GPU";
	for (size_t i = 0; i != nAvailableGPUs; i++)
	{
		SolverGPU* solver = SolverGPUFactory::makeRawNewInstance(_solverName);
		if (solver != nullptr) {
            solver->SetupGPU(i);
            nGPUThreads++;
			worksizeGPU += solver->nTasksToRequest;
			CreateWorker(solver);
		}
		else {
			break;
		}
	}
	nTasksFillCapacity += worksizeGPU;
#endif

	LOG(INFO) << "********************************";
	LOG(INFO) << "   Running with " << nThreads <<" threads  ";
#ifdef GPU_SUPPORT
	LOG(INFO) << "   Running with " << nGPUThreads << " GPUs " << endl;
#endif
	LOG(INFO) << "********************************";

}

void ThreadedTaskProcessor::Update()
{
	// Properly join unused threads
	size_t th_id;
	while (threadsToJoin.try_dequeue(th_id))
	{
		LOG(INFO) <<( "Deactivating thread #" +to_string( th_id));
		JoinThread(th_id);
	}

	if (terminateWhenDone && (nCompletedTasks > nEnqueuedTasks-nThreads)) {
		TerminateAllWorkersWhenDone();
	}
}

// Take a task from dispatchedTasks to execute it (called by a worker)
std::vector<std::unique_ptr<TaskData>> ThreadedTaskProcessor::GetDispatchedTasks(size_t th_id, size_t maxTasks)
{
	std::vector<std::unique_ptr<TaskData>> tasks;
	for (int i=0; i<maxTasks; i++) {
		tasks.emplace_back(std::unique_ptr<TaskData>(nullptr));
	}

	size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk( std::make_move_iterator(tasks.begin()),
	                                                       maxTasks);
	tasks.resize(dequeuedTasks);
	return tasks;
}

void ThreadedTaskProcessor::GiveResults(size_t th_id, std::vector<std::unique_ptr<TaskResults>>&& res)
{
	workerCompletedTasks[th_id] += res.size();
	if (_consumer != nullptr)
	{
		_consumer->EnqueueTasks(std::move(res), workerProducerID[th_id]);
	}
}


// Utility Methods
void ThreadedTaskProcessor::Terminate() {
	TerminateAllWorkers();
}

void ThreadedTaskProcessor::TerminateWhenDone() {
	terminateWhenDone = true;
}

void ThreadedTaskProcessor::ReportAverageSpeed(float speed) {
	return;
}

// Thread managing
void ThreadedTaskProcessor::CreateWorker(Solver* solver)
{
	size_t _id;
	if (!unactivatedThreadPool.empty())
	{
		_id  = unactivatedThreadPool.front();
		unactivatedThreadPool.pop();
	}
	else
	{
		_id = nextWorkerId;
		++nextWorkerId;
	}
	activeThreads.push_back(_id);
	WorkerThread * worker = new WorkerThread(_id, this, solver);
	workers.push_back(worker);
	threads.push_back(thread(&WorkerThread::WorkerLoop, worker));
	workerCompletedTasks.push_back(0);
	if (_consumer != nullptr) {
		workerProducerID.push_back(_consumer->RequestProducerID());
	}
}

void ThreadedTaskProcessor::TerminateAllWorkers()
{
	for (size_t i=0; i!=activeThreads.size(); i++)
	{
		TerminateWorker(activeThreads[i]);
	}
}

void ThreadedTaskProcessor::TerminateAllWorkersWhenDone()
{
	for (size_t i=0; i!=activeThreads.size(); i++)
	{
		TerminateWorkerWhenDone(activeThreads[i]);
	}
}

void ThreadedTaskProcessor::TerminateWorker(size_t i) {
	workers[i]->Terminate();
}
void ThreadedTaskProcessor::TerminateWorkerWhenDone(size_t i) {
	workers[i]->TerminateWhenDone();
}

void ThreadedTaskProcessor::ReportThreadTermination(size_t th_id)
{
	threadsToJoin.enqueue(th_id);
}

void ThreadedTaskProcessor::JoinThread(size_t th_id)
{
	threads[th_id].join();
  delete workers[th_id];
  LOG(INFO) << "deactivated id. "<< th_id <<"Cleaning up it's remains... " << th_id;

	if (_consumer != nullptr)
		_consumer->ReportProducerTermination({workerProducerID[th_id]});

	unactivatedThreadPool.push(th_id);
  activeThreads.erase(std::remove(activeThreads.begin(), activeThreads.end(), th_id), activeThreads.end());
}

size_t ThreadedTaskProcessor::NumberOfCompletedTasks() const{
	nCompletedTasks = std::accumulate(workerCompletedTasks.begin(), workerCompletedTasks.end(),0);
	return nCompletedTasks;
}

float ThreadedTaskProcessor::Progress() const {
    float total = std::accumulate(workerCompletedTasks.begin(), workerCompletedTasks.end(),0);
    for (auto id : activeThreads) {
        total += workers[id]->GetSimulationProgress();
    }
    return total;
}

