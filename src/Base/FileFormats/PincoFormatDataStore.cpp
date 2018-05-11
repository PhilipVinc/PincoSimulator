//
//  PincoFormatDataStore.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "PincoFormatDataStore.hpp"

#include "ChunkFileSet.hpp"
#include "ChunkRegister.hpp"
#include "../TaskResults.hpp"

#include <algorithm>
#include <memory>


using namespace std;


PincoFormatDataStore::PincoFormatDataStore(const Settings* settings,
                                           std::string folderName) :
    DataStore(settings, folderName)
{
    chunkRootPath = dataStoreBasePath + "/";
    cRegister = new ChunkRegister(chunkRootPath + "_register.bin");
    chunkIds = cRegister->GetUsedChunkIds();
    
    cachedWriteChunk = nullptr;
}

PincoFormatDataStore::~PincoFormatDataStore()
{
    for (auto it=chunkFileSets.begin(); it != chunkFileSets.end(); it++)
    {
        delete it->second;
    }
    delete cRegister;
}

size_t PincoFormatDataStore::NewChunkId()
{
    size_t maxUsedId = *std::max_element(chunkIds.begin(), chunkIds.end());
    maxUsedId ++;
    
    return maxUsedId;
}

void PincoFormatDataStore::CreateNewChunk()
{
    size_t id = NewChunkId();
    ChunkFileSet* chunk = new ChunkFileSet(chunkRootPath , datasetN, id);
    chunk->SetMinChunkSize(idealFileSize);

    chunkIds.insert(id);
    chunkFileSets[id] = chunk;
    
    cachedWriteChunk = chunk;
}

ChunkFileSet* PincoFormatDataStore::GetWritableChunk()
{
    if (cachedWriteChunk != nullptr && !cachedWriteChunk->IsChunkBig())
    {
        return cachedWriteChunk;
    }
    
    CreateNewChunk();
    return cachedWriteChunk;
}

ChunkFileSet* PincoFormatDataStore::Chunk(size_t chunkId) {
    if (chunkIds.count(chunkId) == 0) {
        cerr << "ERROR" << endl;
    } // else

    if (chunkFileSets.count(chunkId) == 1) {
        return chunkFileSets[chunkId];
    } else {
        ChunkFileSet* chunk = new ChunkFileSet(chunkRootPath , datasetN, chunkId);
        chunkFileSets[chunkId] = chunk;
        return chunk;
    }
}


void PincoFormatDataStore::Initialise(std::unique_ptr<TaskResults> const& results) {
  datasetN = results->NumberOfDataSets();
  if (!cRegister->registerInitialized) { cRegister->InitializeRegisterHeader(results);}
  initialised = true;
}

void PincoFormatDataStore::StoreDataSimple(std::unique_ptr<TaskResults> const& results)
{
  if (!initialised) { Initialise(results);};

  ChunkFileSet * cnk = GetWritableChunk();
  size_t offset = cnk->WriteToChunk(results, DatasetNamesInRegister());
  cRegister->RegisterStoredData(results, cnk->GetId(), offset, Settings::SaveSettings::saveIdFiles);
}

void PincoFormatDataStore::StoreData(std::unique_ptr<TaskResults> const& results)
{
  if (!initialised) { Initialise(results);};

  ChunkFileSet * cnk = GetWritableChunk();
  size_t offset = cnk->WriteToChunk(results, DatasetNamesInRegister());
  cRegister->RegisterStoredData(results, cnk->GetId(), offset);
}

void PincoFormatDataStore::ProvideDatasetNames(vector<string> names)
{
    datasetN = names.size();
}

void PincoFormatDataStore::LoadListOfStoredDataEvents()
{
    // TODO
    return;
}

const std::set<size_t>& PincoFormatDataStore::UsedIds()
{
    return cRegister->GetSavedTasksIds();
}

std::unique_ptr<TaskResults> PincoFormatDataStore::LoadResult(size_t trajId, bool lastFrameOnly)
{
    if (!initialised)
        datasetN = cRegister->datasetNames.size();
    std::unique_ptr<TaskResults> result(nullptr); //TODO*/

    if (lastFrameOnly) {
        ChunkRegister::RegisterEntry *regEntry = cRegister->GetFinalEntryById(trajId);

        ChunkFileSet *cnk = Chunk(regEntry->chunk_id);

        std::vector<std::tuple<void *, size_t, size_t>> data = cnk->ReadEntry(regEntry->chunk_offset, lastFrameOnly);
        result = ResultsFactory::makeUniqueNewInstance("TWMC"); //TODO
        result->SetId(regEntry->traj_id);
        result->DeSerializeExtraData(regEntry->additionalData, 2); //TODO hardcoded 2

        int i = 0;
        for (auto d : data) {
            result->AddDataset(cRegister->datasetNames[i],
                               std::make_tuple<const void *, size_t>(std::get<0>(d), std::move(std::get<1>(d))),
                               std::get<2>(d), cRegister->dimensionalityData[i]); //TODO*/
            i++;
            delete[] static_cast<char *>(std::get<0>(d));
        }
    } else {
      std::vector<ChunkRegister::RegisterEntry*> entries = cRegister->GetEntryById(trajId);

        for (auto entry: entries) {
          ChunkFileSet *cnk = Chunk(entry->chunk_id);
          std::vector<std::tuple<void *, size_t, size_t>> data = cnk->ReadEntry(entry->chunk_offset);
          std::unique_ptr<TaskResults> newResult = ResultsFactory::makeUniqueNewInstance("TWMC"); //TODO
          newResult->SetId(entry->traj_id);
          newResult->DeSerializeExtraData(entry->additionalData, 2); //TODO hardcoded 2
          int i = 0;
          for (auto d : data) {
            newResult->AddDataset(cRegister->datasetNames[i],
                               std::make_tuple<const void *, size_t>(std::get<0>(d), std::move(std::get<1>(d))),
                               std::get<2>(d), cRegister->dimensionalityData[i]); //TODO*/
            i++;
            delete[] static_cast<char *>(std::get<0>(d));
          }
          if (result)
            result->AppendResult(std::move(newResult));
          else
            result = std::move(newResult);
        }
    }
    return result;
}

