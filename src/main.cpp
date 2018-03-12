
#include "Base/Settings.hpp"
#include "Base/Manager.hpp"
#include "Base/TaskResults.hpp" // For very weird bug

#include <iostream>
#include <csignal>
#include <unistd.h>

#ifdef MPI_SUPPORT
#include <mpi.h>
#include "Base/MPITaskProcessor/MPINodeManager.hpp"
#endif

using namespace std;

int main(int argc, char * argv[])
{
#ifdef MPI_SUPPORT
    int threadLevel;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threadLevel);

    int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int world_size; MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    std::cout << "I am process " << rank << " of " << world_size << std::endl;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    printf("PID %d on %s ready for attach\n", getpid(), hostname);

    fflush(stdout);

    if (rank == 0)
    {
        sleep(3);

        //cout << "Max Tag number is: "<< env.max_tag() << endl;
        //cout << "Reserved tag id is #" << env.collectives_tag() << endl;
        cout << "Thread support level is: "<< threadLevel << endl;
#endif
        Settings *settings = new Settings(argc, argv);

        Manager *manager = ManagerFactory::makeRawNewInstance(settings->get<string>("Manager"), settings);
#ifdef MPI_SUPPORT
        manager->SetMPICommunicator(nullptr);
#endif
        manager->Setup();
        if (!(manager == nullptr)) {
            manager->ManagerLoop();
        } else {
            cout << "Invalid Manager. Exiting..." << endl;
        }

        delete manager;
#ifdef MPI_SUPPORT
    } else {
        int i = 0;
       // while (0 == i)
             sleep(3);
        cout << rank << " - Creating NodeManager" << endl;

        MPINodeManager* node = new MPINodeManager(nullptr);
        node->ManagerLoop();
    }
    MPI_Finalize();
#endif
    return 0;
}

