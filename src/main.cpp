
#include "Base/Settings.hpp"
#include "Base/Manager.hpp"

#include <iostream>
#include <csignal>


#ifdef MPI_SUPPORT
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include "Base/MPIManager/MPINodeManager.hpp"
namespace mpi = boost::mpi;
#endif

using namespace std;

int main(int argc, char * argv[])
{
#ifdef MPI_SUPPORT
    mpi::environment env(boost::mpi::threading::level::funneled);
    mpi::communicator world;
    std::cout << "I am process " << world.rank() << " of " << world.size()
              << " with comm #"<< &world << "."<< std::endl;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    printf("PID %d on %s ready for attach\n", getpid(), hostname);

    fflush(stdout);

    if (world.rank() == 0)
    {
        cout << "Max Tag number is: "<< env.max_tag() << endl;
        cout << "Reserved tag id is #" << env.collectives_tag() << endl;
        cout << "Thread support level is: "<< env.thread_level() << endl;
#endif
        int i = 0;
        // while (0 == i)
        //     sleep(5);

        Settings *settings = new Settings(argc, argv);

        Manager *manager = ManagerFactory::makeRawNewInstance(settings->get<string>("Manager"), settings);
#ifdef MPI_SUPPORT
        cout << "here is " << &world << endl;
        manager->SetMPICommunicator(&world);
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
             sleep(1);

        MPINodeManager* node = new MPINodeManager(&world);
        node->ManagerLoop();
    }
#endif
    return 0;
}

