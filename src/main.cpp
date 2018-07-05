
#include "Base/Settings.hpp"
#include "Base/Manager.hpp"
#include "Base/TaskResults.hpp" // For very weird bug
#include "easylogging++.h"

#include <iostream>
#include <memory>
#include <csignal>
#include <unistd.h>

#ifdef MPI_SUPPORT
#include <mpi.h>
#include "Base/TaskProcessors/MPITaskProcessor/MPINodeManager.hpp"
#endif

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int argc, char * argv[])
{
#ifdef MPI_SUPPORT
    int threadLevel;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threadLevel);
#endif
    START_EASYLOGGINGPP(argc, argv);
    Settings *settings = new Settings(argc, argv);

#ifdef MPI_SUPPORT

    int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int world_size; MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    std::cout << "I am process " << rank << " of " << world_size << std::endl;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    printf("PID %d on %s ready for attach\n", getpid(), hostname);

    fflush(stdout);

    if (rank == 0)
    {
        sleep(6);

        cout << "Thread support level is: "<< threadLevel << endl;
#endif
        std::shared_ptr<Manager> manager = ManagerFactory::makeSharedNewInstance(settings->get<string>("Manager"), settings);
#ifdef MPI_SUPPORT
        manager->SetMPICommunicator(nullptr);
#endif
        manager->Setup();
        if (!(manager == nullptr)) {
            manager->ManagerLoop();
        } else {
            LOG(INFO) << settings->get<string>("Manager") << " - is an invalid Manager. Exiting...";
        }
        delete settings;

#ifdef MPI_SUPPORT
    } else {
        sleep(3);
        LOG(INFO) << rank << " - Creating NodeManager" << endl;
        int ppnMPI = settings->get<int>("ppn", -1);
        if (ppnMPI == -1) {
          ppnMPI = settings->get<int>("processes", -1);
        }
        std::shared_ptr<MPINodeManager> node = std::make_shared<MPINodeManager>(nullptr, ppnMPI);
        node->Setup();
        node->ManagerLoop();
    }
    LOG(INFO) << "#" << rank << " Calling MPI_Finalize()";
    MPI_Finalize();
    LOG(INFO) << "#" << rank << " Completed MPI_Finalize()";
#else
#endif
    return 0;
}

