//
// Created by Filippo Vicentini on 30/01/18.
//

#include "MPINodeManager.hpp"

#include "MPIPincoTags.hpp"

#include "Base/FileFormats/DataStore.hpp"
#include "../ThreadedTaskProcessor/ThreadedTaskProcessor.hpp"
#include "Base/TaskResults.hpp"
#include "easylogging++.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "Base/Serialization/SerializationArchiveFormats.hpp"

#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>


using namespace std;


MPINodeManager::MPINodeManager(MPI_Comm* _comm, int processesPerNode)  {
    comm = _comm;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    LOG(INFO) << "#"<<rank<<" - MPINodeManager Created." << comm ;

    MPI_Status status;
    MPI_Probe(MASTER_RANK, SOLVER_STRING_MESSAGE_TAG, MPI_COMM_WORLD, &status);
    int stringLen; MPI_Get_count(&status, MPI_CHAR, &stringLen);
    char* buf = new char[stringLen];
    MPI_Recv(buf,stringLen,MPI_CHAR, status.MPI_SOURCE,status.MPI_TAG,
    MPI_COMM_WORLD, &status);

    _solverName = std::string(buf, stringLen); delete[] buf;

    LOG(INFO) << "#"<<rank<<" - Received solver:"<< _solverName;

    _processor = new ThreadedTaskProcessor(_solverName, processesPerNode, processesPerNode);
    _processor->SetConsumer(this);
    _processor->Setup();
    LOG(INFO) << "#"<<rank<<" - Done";
    while(tmpTasksToSend.size() < IDEALSIZE) {
        tmpTasksToSend.emplace_back(nullptr);
    }
}


void MPINodeManager::ManagerLoop() {
  chrono::system_clock::time_point lastProgressReportTime     = chrono::system_clock::now();
  chrono::system_clock::duration deltaTProgressReport     = chrono::seconds(10);

    while(!quit) {

        // 1 - Get TaskData : Create Async receive requests from master if we have not an async request.
        if (!receiving) {
            // Check if we have something to receive, otherwise, do not receive.
            MPI_Status status; int gotAMessage;
            MPI_Iprobe(MPI_ANY_SOURCE, TASKDATA_VECTOR_MESSAGE_TAG,
                       MPI_COMM_WORLD, &gotAMessage, &status);
            if (gotAMessage) {
                LOG(INFO) << "#" << rank << " - MPINodeManager::ManagerLoop(). - Creating irecv request" ;

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
                LOG(INFO) << "#" << rank << " - MPINodeManager::ManagerLoop(). - Created irecv request";
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

                    LOG(INFO) << "#"<<rank<< " - Received " << commRecvBuffers[i].size() << " bytes ("
                        << _tasks.size() << ") of taskData from Master.";

                    this->_processor->EnqueueTasks(std::move(_tasks));
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
                LOG(INFO) << "#" << rank << " - MPINodeManager::ManagerLoop(). - sending " << tasksInBuffer << " results";

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
                tasksInBuffer = 0;

                LOG(INFO) << "( " << commSendBuffers.back().size()/(1024*1024)<< " Mb)";

                MPI_Request req; commSendRequests.push_back(req);


                MPI_Isend(commSendBuffers.back().c_str(), commSendBuffers.back().size(),
                          MPI_BYTE, MASTER_RANK, TASKRESULTS_VECTOR_MESSAGE_TAG,
                          MPI_COMM_WORLD, &commSendRequests.back());
            }

        }

        // 3 - Test Async result send requests to master
        for (int i=0; i < commSendRequests.size(); i++)
        {
            MPI_Status status; int done;
            MPI_Test(&commSendRequests[i], &done, &status);
            if (done)
            {
                LOG(INFO) << (" - MPINodeManager::ManagerLoop() - Has sent msg #" + to_string(i) +  " to Master.");
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
                MPI_Recv(&tasksToComputeBeforeTerminating, 1, MPI_INT, status.MPI_SOURCE,
                status.MPI_TAG, MPI_COMM_WORLD, &status);
                LOG(INFO) << " - Received Termination message: must compute " << tasksToComputeBeforeTerminating  << " tasks.";
                terminate = true;
            }
        }

        if (terminate  && receivedTasks >= tasksToComputeBeforeTerminating && commRecvRequests.size() == 0)
        {
            _processor->AllProducersHaveBeenTerminated();

            if (sentResults == receivedTasks && commSendRequests.size() == 0)
            {
                LOG(INFO) << "#" << rank << " - MPINodeManager sending Termination message.";
                MPI_Send(&tasksToComputeBeforeTerminating, 1, MPI_INT,
                MASTER_RANK, NODE_TERMINATED_MESSAGE_TAG, MPI_COMM_WORLD);
                quit = true;
                LOG(INFO) << "#" << rank << " - Quitting.";
                break;
            }
        }

        // Report Progress
        auto now                          = chrono::system_clock::now();
        chrono::system_clock::duration dt = now - lastProgressReportTime;
        if (dt > deltaTProgressReport) {
          float progress = _processor->Progress();
          commSendBuffers.push_back("");
          MPI_Request req; commSendRequests.push_back(req);
          LOG(INFO) << "Sending progress " << progress << " to master.";
          MPI_Isend(&progress, 1,
                    MPI_FLOAT, MASTER_RANK, NODE_PROGRESS_MESSAGE_TAG,
                    MPI_COMM_WORLD, &commSendRequests.back());
          lastProgressReportTime = now;
        }
        // If doing nothing then sleep the controlling thread to give space to the workers
        //      !sending                        !receiving
        if (commSendRequests.size() == 0 && commRecvRequests.size() == 0) {
            sleep(1);
        }
    }
}

void MPINodeManager::AllProducersHaveBeenTerminated() {
    terminate = true;
}

