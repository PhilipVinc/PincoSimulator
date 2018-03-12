//
//  PincoFormatDataStore.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef PincoFormatDataStore_hpp
#define PincoFormatDataStore_hpp

#include "DataStore.hpp"


#include <map>
#include <memory>
#include <set>
#include <vector>
#include <stdio.h>

class ChunkRegister;
class ChunkFileSet;
class TaskResults;


class PincoFormatDataStore : public DataStore
{
public:
    PincoFormatDataStore(const Settings* settings, std::string folderName);
    ~PincoFormatDataStore();
    
    virtual void ProvideDatasetNames(std::vector<std::string> names);

protected:
    
    ChunkFileSet* GetWritableChunk();
private:

	virtual void StoreDataSimple(std::unique_ptr<TaskResults> const& results);
	virtual void StoreDataComplex(std::unique_ptr<TaskResults> const& results);
	virtual void LoadListOfStoredDataEvents();

	//    TaskResults* LoadTaskResults(size_t id,
//                                         TaskResults * res = nullptr);
//    TaskResults* LoadTaskResultsLastNFrames(size_t id,
//                                            size_t nFrames,
//                                            TaskResults *res = nullptr);


	void Initialise(std::unique_ptr<TaskResults> const& task);

	size_t NewChunkId();
	void CreateNewChunk();
	ChunkFileSet* Chunk(size_t id);

	size_t datasetN;
	std::string chunkRootPath;

	std::set<size_t> chunkIds;
    std::map<size_t, ChunkFileSet*> chunkFileSets;
    
    ChunkRegister* cRegister;
    ChunkFileSet* cachedWriteChunk;

	bool initialised = false;
};

#endif /* PincoFormatDataStore_hpp */
