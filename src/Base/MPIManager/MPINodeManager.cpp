//
// Created by Filippo Vicentini on 30/01/18.
//

#include "MPINodeManager.hpp"

#include "MPIPincoTags.hpp"

#include "../FileFormats/DataStore.hpp"
#include "../../ThreadedManager/ThreadedTaskProcessor.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/archive/binary_oarchive.hpp>


using namespace std;


MPINodeManager::MPINodeManager(mpi::communicator* _comm) : comm(_comm)  {

    tmpTasksToSend = std::vector<TaskResults*>(1024,NULL);

    cout << "#"<<comm->rank()<<" - MPINodeManager Created." << comm << endl;

    cout << "#"<<comm->rank()<<" - receiving from master." << comm << endl;
    comm->recv(MASTER_RANK, SOLVER_STRING_MESSAGE_TAG,_solverName);
    cout << "#"<<comm->rank()<<" - Received solver:"<< _solverName << comm << endl;

    cout << "#"<<comm->rank()<<" - Initializing the TaskProcessor-ParallelSolver" <<endl;
    _processor = new ThreadedTaskProcessor(_solverName, -1, -1);
    _processor->SetConsumer(this);
    _processor->Setup();
    cout << "#"<<comm->rank()<<" - Done" <<endl;
}


void MPINodeManager::ManagerLoop() {

    while(!quit) {
        cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop()." << endl;

        //size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk( tmpTasksToSend.begin(), 1024);
        //tmpTasksToSend.resize(dequeuedTasks);

        // self->SendData();

        //tmpTasksToSend.resize(1024);

        // 1 - Get TaskData : Create Async receive requests from master

        /*
        const size_t maxTasks = 1024;
        std::vector<TaskData *> *tasks = new std::vector<TaskData *>(maxTasks, NULL);

        if (!receiving) {
        cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Creating irecv request"
             << endl;
            commRecvRequests.push_back(comm->irecv(MASTER_RANK, TASKDATA_VECTOR_MESSAGE_TAG, *tasks));
            commRecvBuffers.push_back(tasks);

            mpi::test_all(commRecvRequests.begin(), commRecvRequests.end());
            cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Created irecv request"
             << endl;
            cout << "#" << comm->rank() << " - Testing all isend." << endl;
            receiving = true;
        }*/

        // 2 - Get TaskData : Test Async receive requests from master and add them
        for (int i = 0; i < commRecvRequests.size(); i++)
        {
            cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Testing recv req."
                 << endl;
            auto res = commRecvRequests[i].test();
            if (res) {
                cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Has received " <<
                      commRecvBuffers[i]->size()<<" tasks. Enqueing..."  << endl;

                receivedTasks += commRecvBuffers[i]->size();

                _processor->EnqueueTasks(*commRecvBuffers[i]);
                commRecvRequests.erase(commRecvRequests.begin() + i);
                commRecvBuffers.erase(commRecvBuffers.begin() + i);
                i--;
                cout << "#" << comm->rank() << " - Done."<< endl;
                receiving = false;
            }
        }

        // 3 - Dequeue Results that were provided by the Solvers and send them
        size_t dequeuedResults = 0;
        size_t IDEALSIZE = 102;
        if (commSendRequests.size() == 0)
             dequeuedResults = enqueuedTasks.try_dequeue_bulk( tmpTasksToSend.begin() + tasksInBuffer,
                                                               IDEALSIZE - tasksInBuffer);
        if (dequeuedResults != 0)
        {
            tasksInBuffer += dequeuedResults;
            totalDequeued += dequeuedResults;

            if ( tasksInBuffer == IDEALSIZE || tasksInBuffer >= receivedTasks - sentResults )
            {
                cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - sending " << tasksInBuffer << " results." << endl;

                std::vector<TaskResults*> *data = new std::vector<TaskResults*>(tmpTasksToSend.begin(),
                                                                                tmpTasksToSend.begin()+tasksInBuffer);
                sentResults += tasksInBuffer;

                // Encode the data in a buffer and free the memory
                auto t1 = MPI_Wtime();
                std::ostringstream *oss = new std::ostringstream();
                {
                    boost::archive::binary_oarchive oa(*oss);
                    oa << data;
                }
                for (int j=0; j< data->size(); j++)
                {
                    delete (*data)[j];
                }
                delete data;

                auto t2 = MPI_Wtime();
                tasksInBuffer = 0;

                commSendBuffers.push_back(oss);
                MPI_Request *iReq = new MPI_Request;
                commSendRequests.push_back(iReq);


                MPI_Isend(oss->str().c_str(), oss->str().size(),
                          MPI_BYTE, MASTER_RANK, TASKRESULTS_VECTOR_MESSAGE_TAG,
                          *comm, iReq);
            }

        }

        // 3 - Test Async result send requests to master
        for (int i=0; i < commSendRequests.size(); i++)
        {
            //cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop() - Testing send req #" << i << endl;
            MPI_Status status; int done;
            MPI_Test(commSendRequests[i], &done, &status);
            if (done)
            {
                //cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop() - Has sent #" << i << endl;
                delete commSendBuffers[i];
                commSendBuffers.erase(commSendBuffers.begin()+i);
                commSendRequests.erase(commSendRequests.begin()+i);
                i--;
            }
        }

        // 5 - Receive Miscellaneuous Messages
        if (auto message = comm->iprobe())
        {
            auto msg = *message;
            cout << "#" << comm->rank() << " - I've got a message with tag: " << msg.tag() << endl;
            // Check the tag
            if (msg.tag() == TERMINATE_WHEN_DONE_MESSAGE_TAG)
            {
                cout << "#" << comm->rank() << " - Received Termination message." << endl;
                comm->recv(msg.source(), msg.tag(), tasksToComputeBeforeTerminating);
                terminate = true;
            }

            else if (msg.tag() == TASKDATA_VECTOR_MESSAGE_TAG)
            {
                if (!receiving) {

                    const size_t maxTasks = 1024;
                    std::vector<TaskData *> *tasks = new std::vector<TaskData *>(maxTasks, NULL);

                    cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Creating Data irecv request."
                         << endl;
                    commRecvRequests.push_back(comm->irecv(msg.source(), TASKDATA_VECTOR_MESSAGE_TAG, *tasks));
                    commRecvBuffers.push_back(tasks);
                    //cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Created Data irecv request." << endl;

                    mpi::test_all(commRecvRequests.begin(), commRecvRequests.end());
                    //cout << "#" << comm->rank() << " - Testing all isend." << endl;
                    receiving = true;
                }

            }

        }

        if (terminate == true && receivedTasks >= tasksToComputeBeforeTerminating && commRecvRequests.size() == 0)
        {
            _processor->AllProducersHaveBeenTerminated();

            if (sentResults == receivedTasks && commSendRequests.size() == 0)
            {
                cout << "#" << comm->rank() << " - MPINodeManager sending Termination message." << endl;
                comm->send(MASTER_RANK, NODE_TERMINATED_MESSAGE_TAG);
                quit = true;
                cout << "#" << comm->rank() << " - Quitting." << endl;
                break;
            }
        }

        //usleep(1000);
    }
}

void MPINodeManager::AllProducersHaveBeenTerminated() {
    terminate = true;
}

