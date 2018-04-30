//
//  TaskResults.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 30/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TaskResults.hpp"

#include "Exceptions/NotImplementedException.hpp"

#include <functional>
#include <numeric>
#include <vector>

using namespace std;

TaskResults::TaskResults(const size_t _nOfDatasets) { numberOfDatasets = 0; }

void TaskResults::AddResult(size_t elSize, size_t format,
                            const std::vector<size_t> dimensions) {
  numberOfDatasets++;
  datasetElementSize.push_back(elSize);
  datasetFormat.push_back(format);

  dimensionsOfDatasets.push_back(dimensions.size());
  for (size_t dim : dimensions) { dimensionalityData.push_back(dim); }
}

// --------------------
// Generic Accessor Methods

std::vector<size_t> TaskResults::DatasetDimensions(size_t id) const {
  if (id == 0) {
    return std::vector<size_t>(
        dimensionalityData.begin(),
        dimensionalityData.begin() + dimensionsOfDatasets[id]);
  } else {
    size_t pos = std::accumulate(dimensionsOfDatasets.begin(),
                                 dimensionsOfDatasets.begin() + (id - 1),
                                 size_t(1), std::multiplies<>());
    return std::vector<size_t>(
        dimensionalityData.begin() + pos,
        dimensionalityData.begin() + pos + dimensionsOfDatasets[id]);
  }
}

// ------------------ //
// Serialization and default null methods (to take care when
// the libraries do not implement those. They are virtual anyawy

const unsigned int TaskResults::SerializingExtraDataOffset() const {
  throw NotImplemented();
}

const void* TaskResults::SerializeExtraData() const { throw NotImplemented(); }

void TaskResults::DeSerializeExtraData(void* data, unsigned int length) {
  throw NotImplemented();
}
