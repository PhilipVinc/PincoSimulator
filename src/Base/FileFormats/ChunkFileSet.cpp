//
//  ChunkFileSet.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ChunkFileSet.hpp"

#include "Libraries/FilesystemLibrary.h"
#include "Base/TaskResults.hpp"

#include "easylogging++.h"

#include <iostream>

using namespace std;


ChunkFileSet::ChunkFileSet(std::string _basePath,
                           size_t nDatasets,
                           size_t chunkId) :
basePath(_basePath), N(nDatasets), id(chunkId)
{
    if (nDatasets==0)
        LOG(ERROR) << "ERROR: Initialised ChunkFileSet with 0 datasets!";

    Initialise();
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

    delete[] buffer;
    
    // Delete the register only if all files were deleted
    fclose(registerFile);
    if (deleteRegister == true)
        remove(registerFileName.c_str());
}

void ChunkFileSet::Initialise()
{
    if (initialised)
        return;

    files.reserve(N); fileOffsets.reserve(N); fileSizes.reserve(N);
    fileNames.reserve(N);

	  // Create or Open files with the name that is computed.
    for (size_t i =0; i != N; i++)
    {
	      // filename of the various datasets
        fileNames.push_back(basePath + "variable" + to_string(i) + "_" + to_string(id) + ".cnk");

        // if the file exists alredy
        if (filesystem::exists(fileNames[i]))
        {
            LOG(INFO) << "Opening File: " << fileNames[i];
            files.push_back(fopen(fileNames[i].c_str(), "ab+"));
	        fseek(files[i], 0, SEEK_END);
	        fileSizes.push_back(ftell(files[i]));
	        totalFilesSize += ftell(files[i]);
	        fseek(files[i], 0, SEEK_SET);
        }
        else // If it does not, let's create it
        {
            files.push_back(fopen(fileNames[i].c_str(), "wb+"));
            fileSizes.push_back(0);
        }
    }

	  // Filename of the chunk register
    registerFileName = basePath + "index_" + to_string(id) + ".bin";
    registerTrajSize = sizeof(size_t)*(1+2*N);
    bufferByteSize = (1+2*N);
    buffer = new size_t[bufferByteSize];

    if (filesystem::exists(registerFileName)) {
        registerFile = fopen(registerFileName.c_str(), "ab+");
    } else {
        registerFile = fopen(registerFileName.c_str(), "wb+");
    }

    initialised = true;
}

// To delete
size_t ChunkFileSet::WriteToChunk(std::unique_ptr<TaskResults> const& results)
{
    buffer[0] = results->GetId();
    for (size_t i = 0; i!=N; i++) {
        buffer[1 +i*2] = results->DatasetElements(i);
        buffer[2 + i*2] = WriteDataToChunk(i, results->DatasetGet(i),
                                           results->DatasetByteSize(i));
    }
    fwrite(buffer, 1, registerTrajSize, registerFile);
    fflush(registerFile);
    nTrajWritten ++;
    
    return (nTrajWritten-1) ;
} // End to remove

size_t ChunkFileSet::WriteDataToChunk(size_t datasetId, const void *ptr, size_t dataSize)
{
    size_t start = ftell(files[datasetId]);
    fwrite(&dataSize, 1, sizeof(size_t), files[datasetId]);
    fwrite(ptr, 1, dataSize, files[datasetId]);
    fflush(files[datasetId]);
    size_t end = ftell(files[datasetId]);
    
    fileOffsets[datasetId] = start;
    fileSizes[datasetId] += (end - start + 1);
    totalFilesSize += (end - start + 1);
    
    return start; // return the offset of where this is
}

void ChunkFileSet::FlushData()
{
    for (auto file : files) {
        fflush(file);
    }
    fflush(registerFile);
}

bool ChunkFileSet::IsChunkBig()
{
    return (totalFilesSize > minChunkSize);
}

/*void ChunkFileSet::ReadChunkRegister(size_t entryChunkId)
{
    fseek(registerFile, entryChunkId*bufferByteSize, SEEK_SET);

    if (fread(buffer, 1, bufferByteSize, registerFile) == bufferByteSize)
    {

    }

}
*/

std::vector<std::tuple<void*, size_t, size_t>> ChunkFileSet::ReadEntry(size_t registerIndex, bool lastItems)
{
    fseek(registerFile, registerIndex*registerTrajSize, SEEK_SET);

    std::vector<std::tuple<void*, size_t, size_t>> result;

    if (fread(buffer, 1, registerTrajSize, registerFile) == registerTrajSize) {
        for (size_t i = 0; i != N; i++) {
            size_t nEntries = buffer[1 + i*2];
            size_t dataStartOffset = buffer[2 + i*2];

            // Go to the beginning of the data
            fseek(files[i], dataStartOffset , SEEK_SET);

            // Read how big the data is
            size_t dataSize = 0;
            fread(&dataSize, 1, sizeof(size_t), files[i]);

            // If we only want lastItem of each dataset, modify accordingly
            if (lastItems && (nEntries != 1))
            {
                size_t elSize = dataSize / nEntries;
                fseek(files[i], elSize*(nEntries -1), SEEK_CUR);
                dataSize = elSize;
                nEntries = 1;
            }

            // Read the data
            char *dataset = new char[dataSize];
            fread(dataset, 1, dataSize, files[i]);

            result.emplace_back(std::move(static_cast<void*>(dataset)), std::move(dataSize), std::move(nEntries));
        }
        return result;
    } else {
        // Read operation went badly nullptr
        return result;
    }
}