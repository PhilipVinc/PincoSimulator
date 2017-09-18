//
//  TrajectorySaver.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TrajectorySaver_hpp
#define TrajectorySaver_hpp

#include <stdio.h>
#include "DataSaver.hpp"
class TaskResults;

class TrajectorySaver : public DataSaver
{
public:
    
protected:
    virtual void SaveData(TaskResults* results);
private:
    
};
#endif /* TrajectorySaver_hpp */
