//
// Created by Filippo Vicentini on 30/01/18.
//

#include "MPIProcessor.hpp"

#include "MPIPincoTags.hpp"
#include "Base/TaskResults.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

// serialization archive used
#include "Base/Serialization/SerializationArchiveFormats.hpp"
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>



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
    SendTerminationMessage();
    while (activeNodes.size() != 0 ) {
        Update();
    }
    cout << "Destroyed MPIProcessor" << endl;
}

void MPIProcessor::ProvideMPICommunicator(MPI_Comm* _comm)
{
    comm = _comm;
    cout << "I've got rank: Master" <<endl;

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    for (int i=1; i < world_size; i++)
    {
        cout << "Master - send to " << i << " the solvername." << endl;
        // tag 1 is the solvername.
        MPI_Send(_solverName.c_str(), _solverName.length(), MPI_CHAR,
        i, SOLVER_STRING_MESSAGE_TAG, MPI_COMM_WORLD);
        cout << "Master - send to " << i << " has finished." << endl;

        nodeRank.push_back(i);
        activeNodes.push_back(nodeRank.size()-1);
        tasksSentToNode.push_back(0);
        resultsReceivedFromNode.push_back(0);
        recvListeningToNode.push_back(false);
        commRecvRequests.emplace_back(); // Fill with null requests
        commRecvBuffers.emplace_back(); // Fill with null requests
    }
    nNodes = nodeRank.size();

    maxTasks = 1024*activeNodes.size(); //if (maxTasks==0) { maxTasks = 1024;} nNodes++;
    for (int i=0; i<maxTasks; i++) {
        tasks.emplace_back(std::unique_ptr<TaskData>(nullptr));
    }

}

void MPIProcessor::Setup()
{

}

void MPIProcessor::Update()
{
    // 1 - Send the tasks to all the processors
    //TODO : add consumer token
    size_t dequeuedTasks = enqueuedTasks.try_dequeue_bulk(std::make_move_iterator(tasks.begin()), maxTasks);
    if (dequeuedTasks!=0) {
        cout << "Got " << dequeuedTasks << " tasks to dispatch." << endl;
        tasks.resize(dequeuedTasks);

        int nodes = 0;
        size_t nTasksPerNode = ceil(dequeuedTasks/nNodes);
        for (size_t n = 0; n < dequeuedTasks; n += nTasksPerNode) {
            size_t nn = min(nTasksPerNode, dequeuedTasks - n);
            auto data = std::vector<std::unique_ptr<TaskData>>(std::make_move_iterator(tasks.begin() + n),
                                                               std::make_move_iterator(tasks.begin() + n + nn));

            std::ostringstream oss;
            {
                transmissionOutputArchive oa(oss);
                oa(data);
            }
            commSendBuffers.push_back(oss.str());
            commSendRequests.emplace_back();

            int nodeId = activeNodes[nodes];

            cout << "Master: Sending " << nn << " tasks to #" << nodeRank[nodeId] << endl;
            MPI_Isend(commSendBuffers.back().c_str(), commSendBuffers.back().size(),
                      MPI_BYTE, nodeRank[nodeId], TASKDATA_VECTOR_MESSAGE_TAG,
                      MPI_COMM_WORLD, &commSendRequests.back());
            cout << "Master - Created isend to #" << nodeRank[nodeId] << endl;

            tasksSentToNode[activeNodes[nodeId]] += nn;
            nodes++;
            nodes = nodes % activeNodes.size();

        }
        // Done creating the send requests. Now test them
        cout << "Master - Testing all isend" << endl;

        if (statuses.size() < commSendRequests.size()) {
            while (statuses.size() < commSendRequests.size()) {
                statuses.emplace_back();
            }
        }

        int flag;
        MPI_Testall(commSendRequests.size(), &commSendRequests[0], &flag, &statuses[0]);
    }

    // 2 - Try to receive everything
    for (size_t in=0; in < activeNodes.size(); in++)
    {
        int n = activeNodes[in];

        if (recvListeningToNode[n] != true) {
            MPI_Status status; int gotMessage;
            MPI_Iprobe(nodeRank[n], TASKRESULTS_VECTOR_MESSAGE_TAG, MPI_COMM_WORLD, &gotMessage, &status);
            if (gotMessage) {
                cout << "Master: got message from #" << nodeRank[n] << ". Posting receive";
                int msgSize;
                MPI_Get_count(&status, MPI_BYTE, &msgSize);

                cout << " (" << msgSize/(1024*1024)<< " MB)" << endl;

                commRecvBuffers[n].resize(msgSize);

                MPI_Irecv(&commRecvBuffers[n][0], commRecvBuffers[n].size(), MPI_BYTE, nodeRank[n],
                          TASKRESULTS_VECTOR_MESSAGE_TAG, MPI_COMM_WORLD, &commRecvRequests[n]);
                recvListeningToNode[n] = true;
            }
        }
    }
    // 3 - Check for sent stuff
    if (commSendRequests.size() != 0) {
        //cout << "Master - Checking for Sent stuff." << endl;
        for (int i = 0; i < commSendRequests.size(); i++) {
            //cout << "Master - Checking for req " << i << endl;
            MPI_Status status; int done;
            MPI_Test(&commSendRequests[i], &done, &status);
            if (done)
            {
                //cout << "Master - has sent." << endl;
                commSendBuffers.erase(commSendBuffers.begin()+i);
                commSendRequests.erase(commSendRequests.begin()+i);
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
            MPI_Test(&commRecvRequests[n], &flag, &status);
            if (flag)
            {
                std::istringstream iss(std::move(commRecvBuffers[n]));
                transmissionInputArchive ia(iss);

                //cout << "Master: start decoding" << endl;
                std::vector<std::unique_ptr<TaskResults>> taskss;
                ia(taskss);
                resultsReceivedFromNode[n] += taskss.size();
                cout << "Master: Received "<<commRecvBuffers[n].size()<<" bytes ("<<taskss.size()<<") of results from #" << nodeRank[n] << "." << endl;
                this->_consumer->EnqueueTasks(std::move(taskss));

                recvListeningToNode[n] = false;
            }
        }
    }

    // Test miscellaneous requests
    if (miscSendReqs.size() != 0) {
        //cout << "Master - Checking for Sent stuff." << endl;
        for (int i = 0; i < miscSendReqs.size(); i++) {
            //cout << "Master - Checking for req " << i << endl;
            MPI_Status status; int done;
            MPI_Test(&miscSendReqs[i], &done, &status);
            if (done)
            {
                //cout << "Master - has sent." << endl;
                delete[] miscSendBuffers[i]; //TODO use malloc/free
                miscSendBuffers.erase(miscSendBuffers.begin()+i);
                miscSendReqs.erase(miscSendReqs.begin()+i);
                i--;
            }
        }
    }


    // 5 - Receive Miscellaneuous Messages
    MPI_Status status; int flag;
    MPI_Iprobe(MPI_ANY_SOURCE,NODE_TERMINATED_MESSAGE_TAG, MPI_COMM_WORLD,
               &flag, &status);
    if (flag)
    {
        int nn;
        MPI_Recv(&nn, 1, MPI_INT, status.MPI_SOURCE, NODE_TERMINATED_MESSAGE_TAG,
        MPI_COMM_WORLD, &status);
        NodeTerminated(status.MPI_SOURCE);
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

        int *buf = new int;
        buf[0] = tasksSentToNode[i];

        miscSendBuffers.push_back(buf);
        miscSendReqs.emplace_back();

        MPI_Isend(buf, 1, MPI_INT, nodeRank[i], TERMINATE_WHEN_DONE_MESSAGE_TAG, MPI_COMM_WORLD,
                  &miscSendReqs.back());
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