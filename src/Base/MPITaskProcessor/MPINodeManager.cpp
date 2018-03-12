//
// Created by Filippo Vicentini on 30/01/18.
//

#include "MPINodeManager.hpp"

#include "MPIPincoTags.hpp"

#include "../FileFormats/DataStore.hpp"
#include "../ThreadedTaskProcessor/ThreadedTaskProcessor.hpp"
#include "../TaskResults.hpp"
#include "Libraries/PreAllocator.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

#include "Base/Serialization/SerializationArchiveFormats.hpp"

#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>


using namespace std;


MPINodeManager::MPINodeManager(MPI_Comm* _comm)  {
    comm = _comm;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    cout << "#"<<rank<<" - MPINodeManager Created." << comm << endl;

    MPI_Status status;
    MPI_Probe(MASTER_RANK, SOLVER_STRING_MESSAGE_TAG, MPI_COMM_WORLD, &status);
    int stringLen; MPI_Get_count(&status, MPI_CHAR, &stringLen);
    char* buf = new char[stringLen];
    MPI_Recv(buf,stringLen,MPI_CHAR, status.MPI_SOURCE,status.MPI_TAG,
    MPI_COMM_WORLD, &status);

    _solverName = std::string(buf, stringLen); delete[] buf;

    cout << "#"<<rank<<" - Received solver:"<< _solverName << endl;

    _processor = new ThreadedTaskProcessor(_solverName, -1, -1);
    _processor->SetConsumer(this);
    _processor->Setup();
    cout << "#"<<rank<<" - Done" <<endl;
    while(tmpTasksToSend.size() < IDEALSIZE) {
        tmpTasksToSend.emplace_back(nullptr);
    }
}


void MPINodeManager::ManagerLoop() {

    while(!quit) {

        // 1 - Get TaskData : Create Async receive requests from master if we have not an async request.
        if (!receiving) {
            // Check if we have something to receive, otherwise, do not receive.
            MPI_Status status; int gotAMessage;
            MPI_Iprobe(MPI_ANY_SOURCE, TASKDATA_VECTOR_MESSAGE_TAG,
                       MPI_COMM_WORLD, &gotAMessage, &status);
            if (gotAMessage) {
                cout << "#" << rank << " - MPINodeManager::ManagerLoop(). - Creating irecv request" << endl;

                // Extract the message size.
                int msgSize;
                MPI_Get_count(&status, MPI_BYTE, &msgSize);
                //char *data = new char[msgSize];
                commRecvRequests.emplace_back();
                commRecvBuffers.emplace_back(msgSize, 'a');
                MPI_Irecv(&commRecvBuffers.back()[0], msgSize, MPI_BYTE, status.MPI_SOURCE,
                          TASKDATA_VECTOR_MESSAGE_TAG, MPI_COMM_WORLD, &commRecvRequests.back());

                // Start the receive.
                MPI_Test(&commRecvRequests.back(), &gotAMessage, &status);
                receiving = true;
                cout << "#" << rank << " - MPINodeManager::ManagerLoop(). - Created irecv request"
                     << endl;
            }
        }

        // 2 - Get TaskData : Test Async receive requests from master and add them
        for (int i = 0; i < commRecvRequests.size(); i++) {
            // Test the request to make it advance (requested by MPI)
            int completed; MPI_Status status;
            MPI_Test(&commRecvRequests[i], &completed, &status);
            if (completed) {
                std::istringstream iss(std::move(commRecvBuffers[i]));
                {
                    transmissionInputArchive ia(iss);

                    std::vector<std::unique_ptr<TaskData>> _tasks;
                    ia(_tasks);
                    receivedTasks += _tasks.size();
                    this->_processor->EnqueueTasks(std::move(_tasks));

                    cout << "#"<<rank<< " - Received " << commRecvBuffers[i].size() << " bytes ("
                        << _tasks.size() << ") of taskData from Master." << endl;
                }
                commRecvBuffers.erase(commRecvBuffers.begin()+i);
                commRecvRequests.erase(commRecvRequests.begin()+i);
                receiving = false;
            }
        }

        // 3 - Dequeue Results that were provided by the Solvers and send them
        size_t dequeuedResults = 0;
        if (commSendRequests.size() == 0)
             dequeuedResults = enqueuedTasks.try_dequeue_bulk( tmpTasksToSend.begin() + tasksInBuffer,
                                                               IDEALSIZE - tasksInBuffer);
        if (dequeuedResults != 0)
        {
            tasksInBuffer += dequeuedResults;
            totalDequeued += dequeuedResults;

            if ( tasksInBuffer == IDEALSIZE || tasksInBuffer >= receivedTasks - sentResults )
            {
                cout << "#" << rank << " - MPINodeManager::ManagerLoop(). - sending " << tasksInBuffer << " results";

                std::vector<std::unique_ptr<TaskResults>> data(std::make_move_iterator(tmpTasksToSend.begin()),
                                                               std::make_move_iterator(tmpTasksToSend.begin()+tasksInBuffer));
                sentResults += tasksInBuffer;

                // Encode the data in a buffer and free the memory
                auto t1 = MPI_Wtime();
                std::ostringstream oss;
                {
                    transmissionOutputArchive oa(oss);
                    oa(data);
                }
                commSendBuffers.push_back(oss.str());

                auto t2 = MPI_Wtime();
                //std::cout << "The number of Mbytes taken for an archive of "<< tasksInBuffer<<" elements is " << oss->str().size()/1024/1024 <<
                //" or "<< oss->str().size() << " bytes  - encoded in " << t2-t1 << " s." << endl;
                tasksInBuffer = 0;

                cout << "( " << commSendBuffers.back().size()/(1024*1024)<< " Mb)" << endl;

                MPI_Request req; commSendRequests.push_back(req);


                MPI_Isend(commSendBuffers.back().c_str(), commSendBuffers.back().size(),
                          MPI_BYTE, MASTER_RANK, TASKRESULTS_VECTOR_MESSAGE_TAG,
                          MPI_COMM_WORLD, &commSendRequests.back());
            }

        }

        // 3 - Test Async result send requests to master
        for (int i=0; i < commSendRequests.size(); i++)
        {
            //cout << "#" << rank << " - MPINodeManager::ManagerLoop() - Testing send req #" << i << endl;
            MPI_Status status; int done;
            MPI_Test(&commSendRequests[i], &done, &status);
            if (done)
            {
                cout << "#" << rank << " - MPINodeManager::ManagerLoop() - Has sent #" << i << endl;
                commSendBuffers.erase(commSendBuffers.begin()+i);
                commSendRequests.erase(commSendRequests.begin()+i);
                i--;
            }
        }

        // 5 - Receive Miscellaneuous Messages
        MPI_Status status; int flag;
        MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD,
                   &flag, &status);
        if(flag) {
            if (status.MPI_TAG == TERMINATE_WHEN_DONE_MESSAGE_TAG) {
                cout << "#" << rank << " - Received Termination message." << endl;
                MPI_Recv(&tasksToComputeBeforeTerminating, 1, MPI_INT, status.MPI_SOURCE,
                status.MPI_TAG, MPI_COMM_WORLD, &status);
                terminate = true;
            }
        }

        if (terminate == true && receivedTasks >= tasksToComputeBeforeTerminating && commRecvRequests.size() == 0)
        {
            _processor->AllProducersHaveBeenTerminated();

            if (sentResults == receivedTasks && commSendRequests.size() == 0)
            {
                cout << "#" << rank << " - MPINodeManager sending Termination message." << endl;
                MPI_Status status;
                MPI_Send(&tasksToComputeBeforeTerminating, 1, MPI_INT,
                MASTER_RANK, NODE_TERMINATED_MESSAGE_TAG, MPI_COMM_WORLD);
                quit = true;
                cout << "#" << rank << " - Quitting." << endl;
                break;
            }
        }

        //usleep(1000);
    }
}

void MPINodeManager::AllProducersHaveBeenTerminated() {
    terminate = true;
}

