//
//  TaskData.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TaskData_hpp
#define TaskData_hpp

#include <stdio.h>

#ifdef MPI_SUPPORT
#include "boost/serialization/vector.hpp"
#endif

class TaskData
{
public:
    virtual ~TaskData() {};

protected:
    
private:
#ifdef MPI_SUPPORT
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
#endif
};

#endif /* TaskData_hpp */
