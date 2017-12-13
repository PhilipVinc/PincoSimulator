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

#include <set>
#include <stdio.h>
#include <string>
#include <vector>

class TaskResults;
class ChunkFileSet;


class ChunkRegister
{
public:
    struct RegisterEntry
    {
        size_t traj_id;
        size_t chunk_id;
        size_t chunk_offset;
        size_t continuation_offset;
        size_t* additionalData;

        size_t registerWritePosition;
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

	std::vector<size_t> GetSavedTasksIds();
	RegisterEntry* GetEntryById(size_t id);
	RegisterEntry* GetEntryByPosition(size_t index);

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
	bool registerDataRead = false;
	bool newRun = false;
    unsigned int sizeOfTrajEntry = 0;
    unsigned int initialSizeOfTrajEntry = 0;
    size_t* trajBuffer;
    
    std::vector<RegisterEntry*> entries;
    std::set<size_t> chunkIds;
    
    long int additionalDataBegin;
    long int additionalDataLengthOffset;
    long int trajDataBegin;
    long int trajAdditionalDataSize;
    
    size_t storedEntries = 0;
};


#endif /* ChunkRegister_hpp */
