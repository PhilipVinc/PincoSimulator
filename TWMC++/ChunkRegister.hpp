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

class TaskResults;
class ChunkFileSet;

class ChunkRegister
{
    struct registerEntry
    {
        size_t traj_id;
        size_t chunk_n;
        size_t chunk_offset;
        size_t n_frames;
    };

    
public:
    ChunkRegister(const Settings* settings, std::string path);
    ~ChunkRegister();
    
    
    void RegisterStoredData(TaskResults* results,
                            size_t chunkId,
                            size_t chunkOffset,
                            Settings::SaveSettings saveType =
                                        Settings::SaveSettings::saveIdFiles);
    
    
protected:
    
private:

    void OpenRegister();
    //void AppendToRegister(TaskResults* results);
    
    void AddContinuationToRegister(size_t traj_id, size_t chunk_id,
                                   size_t chunk_offset,
                                   size_t frame0, size_t frameEnd);
    
    void InitializeRegisterHeader(TaskResults* results);
    
    
    // Properties
    const Settings* settings;
    
    std::string registerFilePath;
    FILE* registerFile;
    bool registerInitialized = false;
    unsigned int sizeOfTrajEntry = 0;
    unsigned int initialSizeOfTrajEntry = 0;
    size_t* trajBuffer;
    
    std::vector<registerEntry> entries;

};


#endif /* ChunkRegister_hpp */
