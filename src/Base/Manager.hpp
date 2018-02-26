//
//  Manager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef Manager_hpp
#define Manager_hpp

#include "Libraries/TStaticFactory.h"

#include <stdio.h>
#include <string>

#ifdef MPI_SUPPORT
#include <boost/mpi/communicator.hpp>
namespace mpi = boost::mpi;
#endif


class Settings;
class DataSaver;
class TaskData;
class TaskResults;
class Task;

class Manager
{
public:
    Manager(const Settings* settings);
    virtual ~Manager() {};

    virtual void Setup() {};
    virtual void ManagerLoop() = 0;

protected:
    const Settings* settings;

private:

#ifdef MPI_SUPPORT
public:
    void SetMPICommunicator(mpi::communicator* _comm);
protected:
    mpi::communicator* comm;
#endif


};

typedef Base::TFactory<std::string, Manager, Settings*> ManagerFactory;

#endif /* Manager_hpp */
