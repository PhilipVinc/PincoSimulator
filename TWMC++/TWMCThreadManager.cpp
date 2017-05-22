//
//  TWMCThreadManager.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TWMCThreadManager.hpp"
#include "ManagerFactory.hpp"
#include "TWMCSimulation.hpp"

#include <iostream>

class TWMCThreadManagerBuilder: public ManagerFactory::Builder {
public:
    TWMCThreadManagerBuilder(): ManagerFactory::Builder( "TWMCThread" ) {}
    virtual Manager* build( const Settings* settings ) {
        return new TWMCThreadManager( settings );
    }
};
static TWMCThreadManagerBuilder TWMCBuild;

using namespace std;
TWMCThreadManager::TWMCThreadManager(const Settings* settings) : ThreadManager(settings)
{
    cout << "ciao" << endl;
}

TWMCThreadManager::~TWMCThreadManager()
{
    
}

Task* TWMCThreadManager::PrepareTask(Task* _task)
{
    // TO DO
    return _task;
}

Task* TWMCThreadManager::PrepareTask()
{
    return new TWMCSimulation(settings);
}

void TWMCThreadManager::SaveTask(Task* task)
{
    
}


