//
// Created by Filippo Vicentini on 20/12/2017.
//

#include "ThreadedTaskProcessor.hpp"

#include "Base/Solver.hpp"
#include "Base/TaskData.hpp"
#include "WorkerThread.hpp"

#ifdef GPU_SUPPORT
#include <vexcl/vexcl.hpp>
#include "Base/SolverGPU.hpp"
#endif

#include <numeric>


ThreadedTaskProcessor::ThreadedTaskProcessor(const Settings* settings, std::string solverName) :
		TaskProcessor(settings, solverName)
{
	size_t n = std::thread::hardware_concurrency();
	size_t nset = settings->get<size_t>("processes");
#ifdef GPU_SUPPORT
	nThreads = std::min(n, nset);
#else
    nset = nset == 0? 20 : nset;
    nThreads = (n == 0 ? 20 : std::min(n, nset) );
#endif
	maxProcesses = settings->get<size_t>("max_processes");
	if (maxProcesses == 0)
		maxProcesses = 3*n;
}

ThreadedTaskProcessor::~ThreadedTaskProcessor()
{
	TerminateAllWorkers();
	while(activeThreads.size()!= 0)
	{
		size_t th_id;
		while (threadsToJoin.try_dequeue(th_id))
		{
			cout << "Deactivating thread #" << th_id << endl;
			JoinThread(th_id);
		}
	}
}

void ThreadedTaskProcessor::AllProducersHaveBeenTerminated() {
	this->TerminateWhenDone();
}

void ThreadedTaskProcessor::Setup()
{
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
	int nMaxGPUs = _settings->get<int>("GPUs");
	if (nMaxGPUs  < 0)
		nMaxGPUs = 100000;
	nAvailableGPUs = std::min(nAvailableGPUs, nMaxGPUs);

	size_t worksizeGPU = 0;
	std::string gpuSolver = _solverName+"_GPU";
	for (size_t i = 0; i != nAvailableGPUs; i++)
	{
		SolverGPU* solver = SolverGPUFactory::makeRawNewInstance(_solverName);
        solver->SetupGPU(i);
		if (solver != nullptr) {
			nGPUThreads++;
			worksizeGPU += solver->nTasksToRequest;
			CreateWorker(solver);
		}
	}
	nTasksFillCapacity += worksizeGPU;
#endif

	cout << "********************************" << endl;
	cout << "   Running with " << nThreads <<" threads  " << endl;
#ifdef GPU_SUPPORT
	cout << "   Running with " << nGPUThreads << " GPUs " << endl;
#endif
	cout << "********************************" << endl;

}

void ThreadedTaskProcessor::Update()
{
	// Properly join unused threads
	size_t th_id;
	while (threadsToJoin.try_dequeue(th_id))
	{
		cout << "Deactivating thread #" << th_id << endl;
		JoinThread(th_id);
	}

}

// Take a task from dispatchedTasks to execute it (called by a worker)
std::vector<TaskData*> ThreadedTaskProcessor::GetDispatchedTasks(size_t th_id, size_t maxTasks)
{
	std::vector<TaskData*> tasks = std::vector<TaskData*>(maxTasks, NULL);
	size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk( tasks.begin(), maxTasks);
	tasks.resize(dequeuedTasks);
	return tasks;
}

void ThreadedTaskProcessor::GiveResults(size_t th_id, std::vector<TaskResults*> res)
{
	workerCompletedTasks[th_id] += res.size();
	if (_consumer != nullptr) {
		_consumer->EnqueueTasks(res, workerProducerID[th_id]);
	} else {
		std::for_each(res.begin(), res.end(), std::default_delete<TaskResults>());
	}
}


// Utility Methods
void ThreadedTaskProcessor::Terminate() {
	TerminateAllWorkers();
}

void ThreadedTaskProcessor::TerminateWhenDone() {
	TerminateAllWorkersWhenDone();
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
	if (_consumer != nullptr)
		_consumer->ReportProducerTermination({workerProducerID[th_id]});

	//_consumer->ReportProducerTermination(workerProducerID[th_id]);
	unactivatedThreadPool.push(th_id);
	activeThreads.erase(std::remove(activeThreads.begin(), activeThreads.end(), th_id), activeThreads.end());
}

size_t ThreadedTaskProcessor::NumberOfCompletedTasks() {
	auto total = std::accumulate(workerCompletedTasks.begin(), workerCompletedTasks.end(),0);
	return total;
}

float ThreadedTaskProcessor::Progress() {
    float total = std::accumulate(workerCompletedTasks.begin(), workerCompletedTasks.end(),0);
    for (auto wt : workers) {
        total += wt->GetSimulationProgress();
    }
    return total;
}

