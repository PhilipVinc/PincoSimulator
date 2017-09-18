//
//  ChunkFileSet.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ChunkFileSet.hpp"
#include "TaskResults.hpp"

using namespace std;

ChunkFileSet::ChunkFileSet(std::string basePath,
             const std::vector<std::string>& datasetNames,
             size_t chunkId)
{
    N = datasetNames.size();
    files.reserve(N); fileOffsets.reserve(N); fileSizes.reserve(N);
    fileNames.reserve(N);
    for (size_t i =0; i != N; i++)
    {
        fileNames.push_back(basePath + datasetNames[i] + "_" + to_string(chunkId) + ".cnk");
        files.push_back(fopen(fileNames[i].c_str(), "w+"));
        fileSizes.push_back(0);
    }
    
    registerFileName = basePath + "index_" + to_string(chunkId) + ".bin";
    registerFile = fopen(registerFileName.c_str(), "w+");
    registerTrajSize = sizeof(size_t)*(1+2*N);
    buffer = new size_t[(1+2*N)];
}

ChunkFileSet::~ChunkFileSet()
{
    bool deleteRegister = true;
    for (size_t i =0; i != files.size(); i++)
    {
        fclose(files[i]);
        
        // If we did not save any data, remove the empty file.
        if (fileSizes[i] == 0)
        {
            remove(fileNames[i].c_str());
            continue;
        }
        deleteRegister = false;
    }
    
    // Delete the register only if all files were deleted
    fclose(registerFile);
    if (deleteRegister == true)
        remove(registerFileName.c_str());
}

FILE* ChunkFileSet::GetFile(size_t datasetId)
{
    return files[datasetId];
}

size_t ChunkFileSet::WriteToChunk(TaskResults* results)
{
    buffer[0] = results->GetId();
    for (size_t i = 0; i!=N; i++)
    {
        buffer[1 +i*2] = results->ElementsInDataSet(i);
        buffer[2 + i*2] = WriteToChunk(i, results->GetDataSet(i),
                                       results->DataSetSize(i));
    }
    fwrite(buffer, 1, registerTrajSize, registerFile);
    nTrajWritten ++;
    
    return (nTrajWritten-1) ;
}


size_t ChunkFileSet::WriteToChunk(size_t datasetId, const void * ptr, size_t dataSize)
{
    size_t start = ftell(files[datasetId]);
    fwrite(&dataSize, 1, sizeof(size_t), files[datasetId]);
    fwrite(ptr, 1, dataSize, files[datasetId]);
    fflush(files[datasetId]);
    size_t end = ftell(files[datasetId]);
    
    fileOffsets[datasetId] = start;
    fileSizes[datasetId] += (end - start + 1);
    
    return start;
}

void ChunkFileSet::FlushData()
{
    
}

bool ChunkFileSet::IsChunkBig()
{
    return (fileSizes[0] > minChunkSize);
}
