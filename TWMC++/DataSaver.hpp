//
//  DataSaver.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef DataSaver_hpp
#define DataSaver_hpp

#include "Settings.hpp"
class TaskResults;
class ChunkRegister;
class ChunkFileSet;

#include <stdio.h>
#include <string>


using namespace std;

class DataSaver
{
public:
    DataSaver(const Settings* settings);
    ~DataSaver();
    
    virtual void SaveData(TaskResults* results);

protected:
    string saveBasePath;
    string fileNamePrepend;
    string fileNameAppend;
    Settings::SaveSettings saveStatus;
    void CloseFile(FILE* file);
    
    void CreateFolder(string folder);

    // Root Folder containing all trajectory folders.
    string rootFolder;
    const string trajectoryFolderBaseName = "trajectories";
    std::string chunkRegisterFileName;
private:
    ChunkRegister* chunkRegister;
    size_t currentChunkId = 0;
    ChunkFileSet* chunkCache = NULL;
    
};

#endif /* DataSaver_hpp */
