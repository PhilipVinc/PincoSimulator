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


void PincoFormatDataStore::Initialise(std::unique_ptr<TaskResults> const& results) {
	datasetN = results->NumberOfDataSets();
	initialised = true;
}

void PincoFormatDataStore::StoreDataSimple(std::unique_ptr<TaskResults> const& results)
{
	if (!initialised) { Initialise(results);};
    ChunkFileSet * cnk = GetWritableChunk();
    size_t offset = cnk->WriteToChunk(results);
    cRegister->RegisterStoredData(results, cnk->GetId(), offset, Settings::SaveSettings::saveIdFiles);
}

void PincoFormatDataStore::StoreDataComplex(std::unique_ptr<TaskResults> const& results)
{
	if (!initialised) { Initialise(results);};
	ChunkFileSet * cnk = GetWritableChunk();
    size_t offset = cnk->WriteToChunk(results);
    cRegister->RegisterStoredData(results, cnk->GetId(), offset, Settings::SaveSettings::appendIdFiles);
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
