//
//  Manager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "Manager.hpp"
#include "Settings.hpp"

Manager::Manager(const Settings* _settings)
{
    settings = _settings;
}

#ifdef MPI_SUPPORT
void Manager::SetMPICommunicator(MPI_Comm* _comm)
{
    comm = _comm;
}
#endif
