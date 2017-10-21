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
    ChunkFileSet(std::string basePath,
                 const std::vector<std::string>& datasetNames,
                 size_t chunkId);
    ~ChunkFileSet();
    
    FILE* GetFile(size_t datasetId);
    size_t WriteToChunk(TaskResults* results);
    void FlushData();
    bool IsChunkBig();
    
    size_t GetId();
protected:
    size_t WriteToChunk(size_t datasetId, const void * ptr, size_t dataSize);

private:
    std::vector<FILE*> files;
    std::vector<size_t> fileSizes;
    std::vector<size_t> fileOffsets;
    std::vector<std::string> fileNames;
    
    std::string registerFileName;
    FILE* registerFile;
    size_t registerTrajSize;
    size_t N;
    
    size_t* buffer;
    size_t id;
    size_t nTrajWritten = 0;
};

#endif /* ChunkFileSet_hpp */
