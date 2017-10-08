//
//  ChunkRegister.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ChunkRegister_hpp
#define ChunkRegister_hpp

#include "Settings.hpp"
#include <stdio.h>
#include <string>
#include <vector>
#include <set>

class TaskResults;
class ChunkFileSet;

class ChunkRegister
{
    struct registerEntry
    {
        size_t traj_id;
        size_t chunk_id;
        size_t chunk_offset;
        size_t continuation_offset;
        size_t* additionalData;
    };

    
public:
    ChunkRegister(std::string path);
    ~ChunkRegister();
    
    
    void RegisterStoredData(TaskResults* results,
                            size_t chunkId,
                            size_t chunkOffset,
                            Settings::SaveSettings saveType =
                                        Settings::SaveSettings::saveIdFiles);
    

    size_t GetNumberOfChunks();
    std::set<size_t> GetUsedChunkIds();
    size_t GetNumberOfSavedTasks();
    
protected:
    void GoToCurrentWritePosition();
    void GoToTrajectoryDataBegin();
private:

    // Writing
    bool CreateNewRegisterFile();
    bool OpenRegisterFile();
    size_t CheckRegisterVersion();

    // Reading
    bool ReadRegisterHeader();
    bool ReadRegisterEntries();
    //void AppendToRegister(TaskResults* results);
    
    void AddContinuationToRegister(size_t traj_id, size_t chunk_id,
                                   size_t chunk_offset,
                                   size_t frame0, size_t frameEnd);
    
    void InitializeRegisterHeader(TaskResults* results);
    
    
    // Properties
    
    std::string registerFilePath;
    FILE* registerFile;
    bool registerInitialized = false;
    bool newRun = false;
    unsigned int sizeOfTrajEntry = 0;
    unsigned int initialSizeOfTrajEntry = 0;
    size_t* trajBuffer;
    
    std::vector<registerEntry*> entries;
    std::set<size_t> chunkIds;
    
    long int additionalDataBegin;
    long int additionalDataLengthOffset;
    long int trajDataBegin;
    long int trajAdditionalDataSize;
    
    size_t storedEntries = 0;
};


#endif /* ChunkRegister_hpp */
