//
//  TWMCAppendThreadManager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 31/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCAppendThreadManager_hpp
#define TWMCAppendThreadManager_hpp

#include "ThreadManager.hpp"
class Settings;
class TWMCSimData;

#include <stdio.h>
#include <random>


class TWMCAppendThreadManager : public ThreadManager
{
public:
    TWMCAppendThreadManager(const Settings* settings);
    
    ~TWMCAppendThreadManager();
    
protected:
    virtual Task* PrepareTask(Task* _task);
    virtual Task* PrepareTask();
    //virtual void SaveTask(Task* task);
    virtual void PostUpdate();
    
private:
    
    TWMCSimData* sharedTaskData;
    std::mt19937 seedGenerator;
    
};
#endif /* TWMCAppendThreadManager_hpp */
