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
#include "Base/Serialization/SerializationArchiveFormats.hpp"
#include <cereal/types/polymorphic.hpp>
#include <cereal/access.hpp>
#endif

class TaskData
{
public:
    virtual ~TaskData() {};

protected:
    
private:
#ifdef MPI_SUPPORT
    friend class cereal::access;
    template<class Archive>
    void save(Archive & ar) const
    {
    }
    template<class Archive>
    void load(Archive & ar)
    {
    }
#endif
};

#ifdef MPI_SUPPORT
//CEREAL_REGISTER_TYPE_WITH_NAME(TaskResults, "TaskResults")
namespace cereal {
    template <class Archive>
    struct specialize<Archive, TaskData, cereal::specialization::member_load_save> {};
} // namespace ...

#endif


#endif /* TaskData_hpp */
