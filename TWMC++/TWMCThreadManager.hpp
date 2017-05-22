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

#include <stdio.h>

class Settings;

class TWMCThreadManager : public ThreadManager
{
public:
    TWMCThreadManager(const Settings* settings);
    
    ~TWMCThreadManager();
    
protected:
    virtual Task* PrepareTask(Task* _task);
    virtual Task* PrepareTask();
    virtual void SaveTask(Task* task);

private:
    
};
#endif /* TWMCThreadManager_hpp */
