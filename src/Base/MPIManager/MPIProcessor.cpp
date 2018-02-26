//
// Created by Filippo Vicentini on 30/01/18.
//

#include "MPIProcessor.hpp"

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <iostream>
#include <numeric>
#include <algorithm>


#ifdef MPI_SUPPORT
#include <fstream>
// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "TWMC/TWMCResults.hpp"
#include "MPIPincoTags.hpp"

#endif


namespace mpi = boost::mpi;
using namespace std;

MPIProcessor::MPIProcessor(std::string solverName, int nodes, int processesPerNode) :
        TaskProcessor(solverName),
        nNodes(nodes),
        nProcessesPerNode(processesPerNode)
{
    cout << "Constructed MPIProcessor" << endl;

}

MPIProcessor::~MPIProcessor()
{
    cout << "Destroyed MPIProcessor" << endl;
}

void MPIProcessor::ProvideMPICommunicator(mpi::communicator* _comm)
{
    comm = _comm;
    cout << "Receiving the MPICommunicator #" << comm << endl;
    cout << "I've got rank:" << comm->rank() << endl;
    for (int i=1; i < comm->size(); i++)
    {
        cout << "#" << comm->rank() << " - send to " << i << " the solvername." << endl;
        // tag 1 is the solvername.
        comm->send(i, SOLVER_STRING_MESSAGE_TAG, _solverName);
        cout << "#" << comm->rank() << " - send to " << i << " has finished." << endl;

        nodeRank.push_back(i);
        tasksSentToNode.push_back(0);
        resultsReceivedFromNode.push_back(0);
        recvListeningToNode.push_back(false);
        commRecvRequests.push_back(mpi::request()); // Fill with null requests
        commRecvBuffers.push_back(nullptr); // Fill with null requests
    }
    nNodes = nodeRank.size();
}

void MPIProcessor::Setup()
{

}

void MPIProcessor::Update()
{
    cout << "Main MPI PROCESSOR::Update()" << endl;
    // 1 - Send the tasks to all the processors
    const size_t maxTasks = 1024*nNodes;
    std::vector<TaskData*> tasks = std::vector<TaskData*>(maxTasks, NULL);
    //TODO : add consumer token
    size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk(tasks.begin(), maxTasks);
    if (dequeuedTasks!=0) {
        cout << "Got " << dequeuedTasks << " tasks to dispatch." << endl;
        tasks.resize(dequeuedTasks);

        int nodes = 0;
        size_t nTasksPerNode = ceil(dequeuedTasks/nNodes);
        for (size_t n = 0; n < dequeuedTasks; n += nTasksPerNode) {
            size_t nn = min(nTasksPerNode, dequeuedTasks - n);
            auto data = std::vector<TaskData *>(tasks.begin() + n, tasks.begin() + n + nn);
            commSendBuffers.push_back(data);

            /*std::ofstream ofs("filename");
            {
                boost::archive::text_oarchive oa(ofs);
                oa << data;
            }*/

            cout << "Master: Sending " << nn << " tasks to #" << nodeRank[nodes] << endl;
            commSendRequests.push_back(comm->isend(nodeRank[nodes], TASKDATA_VECTOR_MESSAGE_TAG, data));
            tasksSentToNode[nodes] += nn;
            nodes++;
            nodes = nodes % nodeRank.size();
            cout << "Master - Created isend to #" << nodeRank[nodes] << endl;

        }
        // Done creating the send requests. Now test them
        cout << "Master - Testing all isend" << endl;

        mpi::test_all(commSendRequests.begin(), commSendRequests.end());
    }

    // 2 - Try to receive everything
    for (size_t n=0; n < nodeRank.size(); n++)
    {
        // If we are not listening, start listening
        if(recvListeningToNode[n] != true)
        {
            cout << "Master: Starting to listen for results from #" << nodeRank[n] << "." <<endl;

            const size_t maxTasks = 1024; // maximum number of transmitted tasks
            std::vector<TaskResults*> *tasks = new std::vector<TaskResults*>(maxTasks, NULL); //must be deallocated

            commRecvBuffers[n] = tasks;
            commRecvRequests[n] = comm->irecv(nodeRank[n], TASKRESULTS_VECTOR_MESSAGE_TAG, *tasks);
            recvListeningToNode[n] = true;
            commRecvRequests[n].test();
        }
    }
    // 3 - Check for sent stuff
    cout << "Master - Checking for Sent stuff." <<endl;
    for (int i=0; i < commSendRequests.size(); i++)
    {
        cout << "Master - Checking for req "<< i <<endl;
        auto res = commSendRequests[i].test();
        if (res) {
            cout << "Master - has sent." <<endl;
            commSendRequests.erase(commSendRequests.begin() + i);
            commSendBuffers.erase(commSendBuffers.begin() + i);
            i--;
        }
    }

    // 4 - Check for received stuff
    for (int n=0; n < nodeRank.size(); n++)
    {
        // If we are not listening, start listening
        if (recvListeningToNode[n] == true) {
            cout << "Master: Listening to #"<<nodeRank[n]<< " ."<<endl;
            auto res = commRecvRequests[n].test();

            if (res) {
                cout << "Master: Received "<<commRecvBuffers[n]->size()<<" results from #" << nodeRank[n] << "." << endl;
                resultsReceivedFromNode[n] += commRecvBuffers[n]->size();
                recvListeningToNode[n] = false;
                this->_consumer->EnqueueTasks(*(commRecvBuffers[n]));
                delete commRecvBuffers[n]; //deallocate
            }
        }
    }

    // 5 - Receive Miscellaneuous Messages
    if (auto message = comm->iprobe())
    {
        auto msg = *message;
        cout << "Master - I've got a message with tag: " << msg.tag() << endl;
        if(recvListeningToNode[0]) {
            commRecvRequests[0].wait();
            cout << "Master: Received "<<commRecvBuffers[0]->size()<<" results from #" << nodeRank[0] << "." << endl;
            resultsReceivedFromNode[0] += commRecvBuffers[0]->size();
            recvListeningToNode[0] = false;
            this->_consumer->EnqueueTasks(*(commRecvBuffers[0]));
            delete commRecvBuffers[0]; //deallocate
        }
    }

    if (auto message = comm->iprobe(MPI_ANY_SOURCE, TASKRESULTS_VECTOR_MESSAGE_TAG))
    {
        auto msg = *message;

    }

    while ( auto message = comm->iprobe(MPI_ANY_SOURCE, NODE_TERMINATED_MESSAGE_TAG) )
    {
        auto msg = *message;
        // Check the tag
        if (msg.tag() == NODE_TERMINATED_MESSAGE_TAG)
        {
            comm->recv(msg.source(), msg.tag());
            NodeTerminated(msg.source());
        }
    }

    if (producersHaveBeenTerminated && !waitingNodesToTerminate && dequeuedTasks == 0 )//accumulate(tasksSentToNode.begin(), tasksSentToNode.end(),0)  )
    {
        SendTerminationMessage();
    }

    sleep(1);
}

void MPIProcessor::AllProducersHaveBeenTerminated()
{
    cout << "######### Will be terminating producers ######" << endl;
    producersHaveBeenTerminated = true;
}

void MPIProcessor::SendTerminationMessage()
{
    cout << "--- --- --- --- Sending termination messages." << endl;
    for (int i=0; i < nodeRank.size(); i++)
    {
        cout << "Master: - Sent termination message to #" << nodeRank[i] << " with nTasks = "<< tasksSentToNode[i] <<endl;
        miscSendReqs.push_back(comm->isend(nodeRank[i], TERMINATE_WHEN_DONE_MESSAGE_TAG, tasksSentToNode[i]));
    }
    waitingNodesToTerminate = true;
}

size_t MPIProcessor::NumberOfCompletedTasks() {
    auto total = std::accumulate(resultsReceivedFromNode.begin(), resultsReceivedFromNode.end(),0);
    return total;
}

float MPIProcessor::Progress() {
    return NumberOfCompletedTasks();
}

void MPIProcessor::NodeTerminated(size_t nodeId)
{
    auto it = find(nodeRank.begin(), nodeRank.end(), nodeId);
    size_t n = std::distance( nodeRank.begin(), it );
    nodeRank.erase(it);
}
