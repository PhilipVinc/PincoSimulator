//
// Created by Filippo Vicentini on 30/01/18.
//
#include <sstream>
#include <boost/archive/binary_oarchive.hpp>

#include "MPINodeManager.hpp"
#include "../Settings.hpp"
#include "../FileFormats/DataStore.hpp"
#include "MPIPincoTags.hpp"
#include <iostream>
#include "../../ThreadedManager/ThreadedTaskProcessor.hpp"
#include <fstream>
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

                /*std::ofstream ofs("#"+to_string(comm->rank()) + "data");
                {
                    boost::archive::text_oarchive oa(ofs);
                    oa << *commRecvBuffers[i];
                }*/
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
        if (commSendBuffers.size() == 0)
             dequeuedResults = enqueuedTasks.try_dequeue_bulk( tmpTasksToSend.begin() + tasksInBuffer,
                                                               IDEALSIZE - tasksInBuffer);
        cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop(). - Dequeued " << dequeuedResults << " results." << endl;
        if (dequeuedResults != 0)
        {
            tasksInBuffer += dequeuedResults;

            if ( tasksInBuffer == IDEALSIZE || tasksInBuffer >= receivedTasks - sentResults )
            {

                std::vector<TaskResults*> *data = new std::vector<TaskResults*>(tmpTasksToSend.begin(),
                                                                                tmpTasksToSend.begin()+tasksInBuffer);
                tasksInBuffer = 0;
                sentResults += dequeuedResults;

                std::ostringstream oss;
                boost::archive::binary_oarchive oa(oss);
                oa << data;
                std::cout << "The number of bytes taken for an archive is " << oss.str().size() << "\n";


                commSendBuffers.push_back(data);
                commSendRequests.push_back(comm->isend(MASTER_RANK,TASKRESULTS_VECTOR_MESSAGE_TAG,*data));


                std::ofstream ofs("Result"+to_string(commSendRequests.size()));
                boost::archive::text_oarchive ob(ofs);
                ob << *data ;
            }

        }

        // 3 - Test Async receive requests from master
        for (int i = 0; i < commSendRequests.size(); i++)
        {
            cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop() - Testing send req #" << i << endl;
            auto res = commSendRequests[i].test();
            if (res)
            {
                cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop() - Has sent #" << i << endl;
                auto data = *commSendBuffers[i];
                //cout << "#" << comm->rank() << " - cleaning up " << data.size() << endl;
                for (int j=0; j< data.size(); j++)
                {
                    //cout << "deleting res# "<< data[j]->GetId() << endl;
                    delete data[j];
                }
                commSendBuffers.erase(commSendBuffers.begin()+i);
                commSendRequests.erase(commSendRequests.begin()+i);
                //cout << "#" << comm->rank() << " - MPINodeManager::ManagerLoop() - Has cleaned up #"<< i << endl;
                i--;
            }
        }

        // 5 - Receive Miscellaneuous Messages
        //while ( auto message = comm->iprobe())
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

        sleep(1);
    }
}

void MPINodeManager::AllProducersHaveBeenTerminated() {
    terminate = true;
}

