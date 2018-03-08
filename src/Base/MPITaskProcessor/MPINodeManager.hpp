//
// Created by Filippo Vicentini on 30/01/18.
//

#ifndef SIMULATOR_RESULTSERIALIZER_HPP
#define SIMULATOR_RESULTSERIALIZER_HPP

#include "../Interfaces/IResultConsumer.hpp"
#include "../TaskProcessor.hpp"

#include <mpi.h>
#include <string>
#include <sstream>

class Settings;
class DataStore;


class MPINodeManager : public IResultConsumer
{
public:
    MPINodeManager(MPI_Comm* _comm);
    virtual ~MPINodeManager() {};


    void ManagerLoop();

protected:
    void SaveData(TaskResults *results);

    virtual void AllProducersHaveBeenTerminated();
private:
    MPI_Comm* comm;
    int rank;

    TaskProcessor* _processor;

    size_t receivedTasks = 0;
    size_t sentResults = 0;
    size_t tasksToComputeBeforeTerminating;

    std::vector<std::unique_ptr<TaskResults>> tmpTasksToSend;
    size_t IDEALSIZE = 102;

    size_t tasksInBuffer = 0;
    size_t totalDequeued = 0;

    bool terminate = false;
    bool quit = false;

    std::string _solverName;

    std::vector<MPI_Request> commRecvRequests;
    std::vector<char*> commRecvBuffers;
    std::vector<int> commRecvBuffersSize;

    std::vector<std::string*> commSendBuffers;
    std::vector<MPI_Request> commSendRequests;

    std::vector<TaskData*> recvBuffer;
    bool receiving = false;


};


#endif //SIMULATOR_RESULTSERIALIZER_HPP
