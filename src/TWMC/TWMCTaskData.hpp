//
// Created by Filippo Vicentini on 22/12/17.
//

#ifndef SIMULATOR_TWMCSIMTASKDATA_HPP
#define SIMULATOR_TWMCSIMTASKDATA_HPP

#include "Base/TaskData.hpp"

#include "TWMCSystemData.hpp"

#include <memory>

#ifdef MPI_SUPPORT
#include "Base/Serialization/SerializationArchiveFormats.hpp"

#include <cereal/access.hpp>  // For LoadAndConstruct
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include "Libraries/eigen_cereal_serialization.hpp"
#endif

#include "TWMCResults.hpp"

class TWMCTaskData : public TaskData {
 public:
  TWMCTaskData();
  std::shared_ptr<TWMCSystemData> systemData;
  std::unique_ptr<TWMCResults> prevData;

  size_t id;
  float_p t_start;
  float_p t_end;

  enum InitialConditions { ReadFromSettings, FixedPoint, ReadFromPreviousData };
  bool storeInitialState = true;
  InitialConditions initialCondition;
  unsigned int rngSeed;

 protected:
 private:
#ifdef MPI_SUPPORT
  friend class cereal::access;

  template <class Archive>
  void save(Archive& ar) const {
    ar(cereal::virtual_base_class<TaskData>(this));
    ar(systemData);
    ar(prevData);
    ar(id);
    ar(t_start);
    ar(t_end);
    ar(storeInitialState);
    ar(initialCondition);
    ar(rngSeed);
  }

  template <class Archive>
  void load(Archive& ar) {
    ar(cereal::virtual_base_class<TaskData>(this));
    ar(systemData);
    ar(prevData);
    ar(id);
    ar(t_start);
    ar(t_end);
    ar(storeInitialState);
    ar(initialCondition);
    ar(rngSeed);
  }
#endif
};

#ifdef MPI_SUPPORT
namespace cereal {
template <class Archive>
struct specialize<Archive, TWMCTaskData,
                  cereal::specialization::member_load_save> {};
}  // namespace cereal

CEREAL_REGISTER_TYPE_WITH_NAME(TWMCTaskData, "TWMCTaskData")
CEREAL_REGISTER_POLYMORPHIC_RELATION(TaskData, TWMCTaskData)
#endif

#endif  // SIMULATOR_TWMCSIMTASKDATA_HPP
