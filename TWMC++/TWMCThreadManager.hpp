//
//  TWMCThreadManager.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 18/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TWMCThreadManager_hpp
#define TWMCThreadManager_hpp

#include "ThreadManager.hpp"
class Settings;
class TWMCSimData;

#include <stdio.h>
#include <random> 


class TWMCThreadManager : public ThreadManager
{
public:
    TWMCThreadManager(const Settings* settings);
    
    ~TWMCThreadManager();
    
protected:
    virtual Task* PrepareTask(Task* _task);
    virtual Task* PrepareTask();
    //virtual void SaveTask(Task* task);
    virtual void PostUpdate();

private:
    
    TWMCSimData* sharedTaskData;
    std::mt19937 seedGenerator;

};
#endif /* TWMCThreadManager_hpp */
