//
// Created by Filippo Vicentini on 30/01/18.
//

#ifndef SIMULATOR_MPIMANAGER_HPP
#define SIMULATOR_MPIMANAGER_HPP

#include "../Interfaces/IResultConsumer.hpp"
#include "../TaskProcessor.hpp"

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include <queue>


namespace mpi = boost::mpi;


class MPIProcessor : public TaskProcessor
{
public:
    MPIProcessor(std::string solverName, int nodes, int processesPerNode);
    ~MPIProcessor();

    void ProvideMPICommunicator(boost::mpi::communicator* comm);

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

    mpi::communicator* comm;

    // Nodes and their performance
    std::vector<int> activeNodes;
    std::vector<int> nodeRank;
    std::vector<size_t> tasksSentToNode;
    std::vector<size_t> resultsReceivedFromNode;

    // MPI Send TaskData elements
    std::vector<mpi::request> commSendRequests;
    std::vector<std::vector<TaskData*>> commSendBuffers;

    //MPI Receuve Buffers and flags
    std::vector<bool> recvListeningToNode;
    std::vector<char*> commRecvBuffers;
    std::vector<size_t> commRecvBuffersSize;
    std::vector<MPI_Request*> commRecvRequests;

    std::vector<mpi::request> miscSendReqs;

    bool producersHaveBeenTerminated = false;
    bool waitingNodesToTerminate = false;
};


#endif //SIMULATOR_MPIMANAGER_HPP
