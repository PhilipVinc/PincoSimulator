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

    virtual void ManagerLoop() = 0;

protected:
    const Settings* settings;

private:
    
};

typedef Base::TFactory<std::string, Manager, Settings*> ManagerFactory;

#endif /* Manager_hpp */
