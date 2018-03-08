//
// Created by Filippo Vicentini on 30/01/18.
//

#ifndef SIMULATOR_MPIMANAGER_HPP
#define SIMULATOR_MPIMANAGER_HPP

#include "../Interfaces/IResultConsumer.hpp"
#include "../TaskProcessor.hpp"

#include <mpi.h>

#include <queue>


class MPIProcessor : public TaskProcessor
{
public:
    MPIProcessor(std::string solverName, int nodes, int processesPerNode);
    ~MPIProcessor();

    void ProvideMPICommunicator(MPI_Comm* comm);

    virtual void Update() final;
    virtual void Setup() final;

    void AllProducersHaveBeenTerminated();
    void SendTerminationMessage();

    void NodeTerminated(size_t nodeId);

    virtual size_t NumberOfCompletedTasks() final;
    virtual float Progress() final;

private:

protected:
    int nNodes;
    int nProcessesPerNode;

    MPI_Comm* comm;

    // Dequeue buffer
    size_t maxTasks;
    std::vector<std::unique_ptr<TaskData>> tasks;

    // Nodes and their performance
    std::vector<int> activeNodes;
    std::vector<int> nodeRank;
    std::vector<size_t> tasksSentToNode;
    std::vector<size_t> resultsReceivedFromNode;

    // MPI Send TaskData elements
    std::vector<MPI_Request> commSendRequests;
    std::vector<std::ostringstream*> commSendBuffers;

    //MPI Receuve Buffers and flags
    std::vector<bool> recvListeningToNode;
    std::vector<char*> commRecvBuffers;
    std::vector<size_t> commRecvBuffersSize;
    std::vector<MPI_Request> commRecvRequests;

    // Buffer
    std::vector<MPI_Status> statuses;

    std::vector<MPI_Request> miscSendReqs;
    std::vector<void*> miscSendBuffers;

    bool producersHaveBeenTerminated = false;
    bool waitingNodesToTerminate = false;
};


#endif //SIMULATOR_MPIMANAGER_HPP
