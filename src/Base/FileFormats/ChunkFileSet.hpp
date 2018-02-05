//
//  ChunkFileSet.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ChunkFileSet_hpp
#define ChunkFileSet_hpp

#include <stdio.h>
#include <string>
#include <vector>

class TaskResults;


const size_t minChunkSize = 1024*1024*1024;


class ChunkFileSet
{
public:
	ChunkFileSet(std::string basePath, size_t nDatasets, size_t chunkId);

	~ChunkFileSet();
    
    FILE* GetFile(size_t datasetId);
    size_t WriteToChunk(TaskResults* results);
    void FlushData();
    bool IsChunkBig();
    
    size_t GetId() {return id;};
protected:
    size_t WriteToChunk(size_t datasetId, const void * ptr, size_t dataSize);
    TaskResults* ReadEntry(size_t entryChunkId, bool lastItems = false);
	void Initialise();

private:
    std::vector<FILE*> files;
    std::vector<size_t> fileSizes;
    std::vector<size_t> fileOffsets;
    std::vector<std::string> fileNames;

	std::string basePath;
	std::string registerFileName;
    FILE* registerFile;
    size_t registerTrajSize;
    size_t N;
    
    size_t* buffer;
	size_t bufferByteSize;
    size_t id;
    size_t nTrajWritten = 0;
	bool initialised = false;
};

#endif /* ChunkFileSet_hpp */
