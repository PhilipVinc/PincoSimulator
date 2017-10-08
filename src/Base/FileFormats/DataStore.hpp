//
//  DataStore.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef DataStore_hpp
#define DataStore_hpp

#include <stdio.h>
#include <string>
#include <vector>

#include "Settings.hpp"
#include "CustomTypes.h"

class TaskData;
class TaskResults;

using namespace std;

class DataStore
{
public:
    DataStore(const Settings* settings, std::string folderName);
    virtual ~ DataStore() {};
    bool SaveTaskResults(TaskResults* task);
    bool AppendTaskResults(TaskResults* task);
//    TaskResults* LoadTaskResults(size_t id,
//                                         TaskResults * res = nullptr);
//    TaskResults* LoadTaskResultsLastNFrames(size_t id,
//                                            size_t nFrames,
//                                            TaskResults *res = nullptr);
    
    virtual void ProvideDatasetNames(vector<string> names) = 0;
    void SaveFile(string fileName, vector<float_p> data);
    void SaveFile(string fileName, vector<vector<float_p>> data);
    // Another save
    
protected:
    //virtual void DataOpen() = 0;
    std::string rootFolder;
    std::string dataStoreBasePath;
    
    bool isFileReady = false;
    bool isReadable = false;
    bool isWritable = false;
    
    virtual void StoreDataSimple(TaskResults* results) = 0;

    Settings::SaveSettings saveStatus;
private:
    
    void CreateFolder(string folder);
};


#endif /* DataStore_hpp */
