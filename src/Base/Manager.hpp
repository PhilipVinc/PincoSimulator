//
//  Manager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef Manager_hpp
#define Manager_hpp

#include <stdio.h>
class Settings;
class DataSaver;
class TaskData;
class TaskResults;
class Task;


class Manager
{
public:
    Manager(const Settings* settings);
    ~Manager();
    
    virtual void ManagerLoop() = 0;
    
    // Get the simulation Task Data;
    virtual TaskData* SimulationData() = 0;
    virtual TaskResults* SampleTaskResult() = 0;

protected:
    const Settings * settings;
    DataSaver* saver;
    
    virtual void SaveTask(Task* task);

private:
    
};

#endif /* Manager_hpp */
