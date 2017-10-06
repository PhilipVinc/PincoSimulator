//
//  PincoFormatDataStore.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef PincoFormatDataStore_hpp
#define PincoFormatDataStore_hpp

#include <stdio.h>
#include "DataStore.hpp"
#include <set>
#include <map>
#include <vector>

class ChunkRegister;
class ChunkFileSet;
class TaskResults;

using namespace std;

class PincoFormatDataStore : public DataStore
{
public:
    PincoFormatDataStore(const Settings* settings, std::string folderName);
    ~PincoFormatDataStore();
    
    virtual void ProvideDatasetNames(vector<string> names);

protected:
    
    ChunkFileSet* GetWritableChunk();
private:
    
    virtual void StoreDataSimple(TaskResults* results);
    
    vector<string> datasetNames;
    string chunkRootPath;
    
    size_t NewChunkId();
    void CreateNewChunk();

    
    std::set<size_t> chunkIds;
    std::map<size_t, ChunkFileSet*> chunkFileSets;
    
    ChunkRegister* cRegister;
    ChunkFileSet* cachedWriteChunk;
    ChunkFileSet* Chunk(size_t id);
};

#endif /* PincoFormatDataStore_hpp */
