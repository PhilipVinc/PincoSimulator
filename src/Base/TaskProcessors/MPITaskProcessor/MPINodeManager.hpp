//
// Created by Filippo Vicentini on 30/01/18.
//

#ifndef SIMULATOR_RESULTSERIALIZER_HPP
#define SIMULATOR_RESULTSERIALIZER_HPP

#include "Base/Interfaces/IResultConsumer.hpp"
#include "Base/TaskProcessor.hpp"

#include <memory>
#include <mpi.h>
#include <string>
#include <sstream>

class Settings;
class DataStore;


class MPINodeManager : public IResultConsumer
{
public:
    explicit MPINodeManager(MPI_Comm* _comm, int processesPerNode);
    ~MPINodeManager();

    void Setup();

    void ManagerLoop();

protected:
    virtual void AllProducersHaveBeenTerminated() final;
private:
    MPI_Comm* comm;
    int rank;

    std::unique_ptr<TaskProcessor> _processor;


    size_t receivedTasks = 0;
    size_t sentResults = 0;
    int tasksToComputeBeforeTerminating;

    std::vector<std::unique_ptr<TaskResults>> tmpTasksToSend;
    size_t IDEALSIZE = 102;

    size_t tasksInBuffer = 0;
    size_t totalDequeued = 0;

    bool terminate = false;
    bool quit = false;

    std::string _solverName;

    std::vector<MPI_Request> commRecvRequests;
    std::vector<std::string> commRecvBuffers;

    std::vector<std::string> commSendBuffers;
    std::vector<MPI_Request> commSendRequests;

    bool receiving = false;
};


#endif //SIMULATOR_RESULTSERIALIZER_HPP
