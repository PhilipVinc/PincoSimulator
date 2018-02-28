//
// Created by Filippo Vicentini on 30/01/18.
//

#ifndef SIMULATOR_RESULTSERIALIZER_HPP
#define SIMULATOR_RESULTSERIALIZER_HPP

#include "../Interfaces/IResultConsumer.hpp"
#include "../TaskProcessor.hpp"

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <string>
#include <sstream>

class Settings;
class DataStore;


namespace mpi = boost::mpi;


class MPINodeManager : public IResultConsumer
{
public:
    MPINodeManager(mpi::communicator* _comm);
    virtual ~MPINodeManager() {};


    void ManagerLoop();

protected:
    void SaveData(TaskResults *results);

    virtual void AllProducersHaveBeenTerminated();
private:
    mpi::communicator* comm;

    TaskProcessor* _processor;

    size_t receivedTasks = 0;
    size_t sentResults = 0;
    size_t tasksToComputeBeforeTerminating;

    std::vector<TaskResults*> tmpTasksToSend;
    size_t tasksInBuffer = 0;
    size_t totalDequeued = 0;

    bool terminate = false;
    bool quit = false;

    std::string _solverName;

    std::vector<mpi::request> commRecvRequests;
    std::vector<std::vector<TaskData*>*> commRecvBuffers;
    std::vector<std::ostringstream*> commSendBuffers;
    std::vector<MPI_Request*> commSendRequests;

    std::vector<TaskData*> recvBuffer;
    bool receiving = false;


};


#endif //SIMULATOR_RESULTSERIALIZER_HPP
