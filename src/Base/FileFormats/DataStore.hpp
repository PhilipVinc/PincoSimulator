//
//  DataStore.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef DataStore_hpp
#define DataStore_hpp

#include "../CustomTypes.h"
#include "../Settings.hpp"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>

class TaskData;
class TaskResults;


class DataStore
{
public:
    DataStore(const Settings* settings, std::string folderName);
    virtual ~ DataStore() {};
    bool SaveTaskResults(std::unique_ptr<TaskResults> const& task);
    bool AppendTaskResults(std::unique_ptr<TaskResults> const& task);
//    TaskResults* LoadTaskResults(size_t id,
//                                         TaskResults * res = nullptr);
//    TaskResults* LoadTaskResultsLastNFrames(size_t id,
//                                            size_t nFrames,
//                                            TaskResults *res = nullptr);
    
    virtual void ProvideDatasetNames(std::vector<std::string> names) = 0;
    void SaveFile(std::string fileName, std::vector<float_p> data);
    void SaveFile(std::string fileName, std::vector<std::vector<float_p>> data);
    // Another save
    
protected:
    //virtual void DataOpen() = 0;
    std::string rootFolder;
    std::string dataStoreBasePath;
    
    bool isFileReady = false;
    bool isReadable = false;
    bool isWritable = false;
    
    virtual void StoreDataSimple(std::unique_ptr<TaskResults> const& results) = 0;
    virtual void LoadListOfStoredDataEvents() = 0;

    Settings::SaveSettings saveStatus;
private:
    
    void CreateFolder(std::string folder);
};


#endif /* DataStore_hpp */
