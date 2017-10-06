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
#include "CustomTypes.h"
class TaskResults;
class ChunkRegister;
class ChunkFileSet;
class DataStore;

#include <stdio.h>
#include <string>
#include <vector>


using namespace std;

class DataSaver
{
public:
    DataSaver(const Settings* settings);
    ~DataSaver();
    
    virtual void SaveData(TaskResults* results);
    void ProvideDatasetNames(vector<string> names);

    void SaveFile(string fileName, float_p* data, size_t length);

protected:
    string saveBasePath;
    string fileNamePrepend;
    string fileNameAppend;
    Settings::SaveSettings saveStatus;
    
    // Root Folder containing all trajectory folders.
    string rootFolder;
    
private:
    DataStore* dataStore;
};

#endif /* DataSaver_hpp */
