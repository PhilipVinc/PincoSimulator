//
// Created by Filippo Vicentini on 30/01/18.
//

#include "MPIProcessor.hpp"

#include "MPIPincoTags.hpp"
#include "TWMC/TWMCResults.hpp"

// serialization archive used
#include <boost/archive/binary_iarchive.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>


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
        activeNodes.push_back(nodeRank.size()-1);
        tasksSentToNode.push_back(0);
        resultsReceivedFromNode.push_back(0);
        recvListeningToNode.push_back(false);
        commRecvRequests.push_back(nullptr); // Fill with null requests
        commRecvBuffers.push_back(nullptr); // Fill with null requests
        commRecvBuffersSize.push_back(0); // Fill with null requests
    }
    nNodes = nodeRank.size();
}

void MPIProcessor::Setup()
{

}

void MPIProcessor::Update()
{
    //cout << "Main MPI PROCESSOR::Update()" << endl;
    // 1 - Send the tasks to all the processors
    const size_t maxTasks = 1024*activeNodes.size();
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

            int nodeId = activeNodes[nodes];

            cout << "Master: Sending " << nn << " tasks to #" << nodeRank[nodeId] << endl;
            commSendRequests.push_back(comm->isend(nodeRank[nodeId], TASKDATA_VECTOR_MESSAGE_TAG, data));
            cout << "Master - Created isend to #" << nodeRank[nodeId] << endl;

            tasksSentToNode[nodes] += nn;
            nodes++;
            nodes = nodes % activeNodes.size();

        }
        // Done creating the send requests. Now test them
        cout << "Master - Testing all isend" << endl;

        mpi::test_all(commSendRequests.begin(), commSendRequests.end());
    }

    // 2 - Try to receive everything
    for (size_t in=0; in < activeNodes.size(); in++)
    {
        int n = activeNodes[in];

        if (recvListeningToNode[n] != true) {
            MPI_Status status;
            int flag;
            MPI_Iprobe(nodeRank[n], TASKRESULTS_VECTOR_MESSAGE_TAG, *comm, &flag, &status);
            if (flag) {
                cout << "Master: got message from #" << nodeRank[n] << ". Posting receive" << endl;
                int msgSize;
                MPI_Get_count(&status, MPI_BYTE, &msgSize);
                char *data = new char[msgSize];

                MPI_Request *iReq = new MPI_Request;
                commRecvRequests[n] = iReq;
                commRecvBuffers[n] = data;
                commRecvBuffersSize[n] = msgSize;

                MPI_Irecv(data, msgSize, MPI_BYTE, nodeRank[n], TASKRESULTS_VECTOR_MESSAGE_TAG, *comm, iReq);
                recvListeningToNode[n] = true;
            }
        }
    }
    // 3 - Check for sent stuff
    if (commSendRequests.size() != 0) {
        //cout << "Master - Checking for Sent stuff." << endl;
        for (int i = 0; i < commSendRequests.size(); i++) {
            //cout << "Master - Checking for req " << i << endl;
            auto res = commSendRequests[i].test();
            if (res) {
                //cout << "Master - has sent." << endl;
                commSendRequests.erase(commSendRequests.begin() + i);
                commSendBuffers.erase(commSendBuffers.begin() + i);
                i--;
            }
        }
    }

    // 4 - Check for received stuff (all nodes. also inactive)
    for (size_t n=0; n < nodeRank.size(); n++)
    {
        //int n = activeNodes[in];
        // If we are not listening, start listening
        if (recvListeningToNode[n] == true)
        {
            int flag; MPI_Status status;
            MPI_Test(commRecvRequests[n], &flag, &status);
            if (flag)
            {
                std::istringstream iss(string(commRecvBuffers[n] , commRecvBuffersSize[n]));
                boost::archive::binary_iarchive ia(iss);

                //cout << "Master: start decoding" << endl;
                std::vector<TaskResults*> *taskss = new std::vector<TaskResults*>(maxTasks, NULL); //must be deallocated
                ia >> taskss;
                resultsReceivedFromNode[n] += taskss->size();
                cout << "Master: Received "<<commRecvBuffersSize[n]<<" bytes ("<<taskss->size()<<") of results from #" << nodeRank[n] << "." << endl;
                this->_consumer->EnqueueTasks(*taskss);

                delete[] commRecvBuffers[n];
                recvListeningToNode[n] = false;
            }
        }
    }

    // 5 - Receive Miscellaneuous Messages
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

    //usleep(1000000);
}

void MPIProcessor::AllProducersHaveBeenTerminated()
{
    cout << "######### Will be terminating producers ######" << endl;
    producersHaveBeenTerminated = true;
}

void MPIProcessor::SendTerminationMessage()
{
    cout << "--- --- --- --- Sending termination messages." << endl;
    for (size_t in=0; in < activeNodes.size(); in++)
    {
        int i = activeNodes[in];
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

    for (int i = 0; i < activeNodes.size(); i++)
    {
        if (activeNodes[i] == n)
        {
            activeNodes.erase(activeNodes.begin()+i);
            break;
        }
    }
}
