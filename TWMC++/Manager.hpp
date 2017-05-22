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

class Manager
{
public:
    Manager(const Settings* settings);
    ~Manager();
    
    virtual void ManagerLoop() = 0;
protected:
    const Settings * settings;
private:
    
};

#endif /* Manager_hpp */
