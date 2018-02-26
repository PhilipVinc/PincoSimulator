//
// Created by Filippo Vicentini on 22/12/17.
//

#include "TWMCTaskData.hpp"

TWMCTaskData::TWMCTaskData() : TaskData()
{

}

#ifdef MPI_SUPPORT
//BOOST_CLASS_EXPORT_IMPLEMENT(TWMCTaskData)
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <iostream>
using namespace std;



//namespace boost {
// namespace archive {
// namespace detail {
//     namespace extra_detail {
//        template<>
//        struct init_guid< TWMCTaskData >
//        {
//            static guid_initializer< TWMCTaskData > const & g;
//        };
//    guid_initializer< TWMCTaskData > const & init_guid< TWMCTaskData >::g =
//        ::boost::serialization::singleton<
//            guid_initializer< TWMCTaskData >
//        >::get_mutable_instance().export_guid();
//    }
//  }
// }
//}

/*

template<class Archive> void TWMCTaskData::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<TaskData>(*this);
    ar & t_start;
    ar & t_end;
    ar & initialCondition;
    ar & rngSeed;
    ar & systemData;
}
*/
#endif
