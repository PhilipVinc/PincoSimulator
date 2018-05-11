//
//  ChunkRegister.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ChunkRegister_hpp
#define ChunkRegister_hpp

#include "../Settings.hpp"

#include <set>
#include <unordered_map>
#include <stdio.h>
#include <memory>
#include <string>
#include <vector>

class TaskResults;
class ChunkFileSet;
class PincoFormatDataStore;


class ChunkRegister
{
public:
    struct RegisterEntry
    {
        size_t traj_id;
        size_t chunk_id;
        size_t chunk_offset;
        size_t continuation_offset;
        char* additionalData;

        size_t registerWritePosition;
    };


public:
    ChunkRegister(std::string path);
    ~ChunkRegister();
    
    
    void RegisterStoredData(std::unique_ptr<TaskResults> const& results,
                            size_t chunkId,
                            size_t chunkOffset,
                            Settings::SaveSettings saveType =
                                        Settings::SaveSettings::unspecified);
    

    size_t GetNumberOfChunks();
    std::set<size_t> GetUsedChunkIds();
    size_t GetNumberOfSavedTasks();
    size_t GetNumberOfSavedTrajectories();

	const std::set<size_t>& GetSavedTasksIds();
	RegisterEntry* GetFinalEntryById(size_t id);
	std::vector<RegisterEntry*> GetEntryById(size_t index);
  RegisterEntry* GetEntryByPosition(size_t index);

  std::vector<std::string> datasetNames;
  std::vector<std::vector<size_t>> dimensionalityData;

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

    void AddContinuationToRegister(size_t traj_id, size_t chunk_id,
                                   size_t chunk_offset,
                                   size_t frame0, size_t frameEnd);
    
    void InitializeRegisterHeader(std::unique_ptr<TaskResults> const& results);
    
    
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
    std::unordered_multimap<size_t, size_t> trajOffsets;
    std::set<size_t> chunkIds;
    std::set<size_t> trajIds;
    
    long int additionalDataBegin;
    long int additionalDataLengthOffset;
    long int trajDataBegin;
    long int trajAdditionalDataSize;
    
    size_t storedEntries = 0;

    friend class PincoFormatDataStore;
};


#endif /* ChunkRegister_hpp */
