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
#include <set>
#include <stdio.h>

class TaskData;
class TaskResults;


class DataStore
{
public:
    DataStore(const Settings* settings, std::string folderName);
    virtual ~ DataStore() {};
    bool SaveTaskResults(std::unique_ptr<TaskResults> const& task);
    //bool AppendTaskResults(std::unique_ptr<TaskResults> const& task);
    std::unique_ptr<TaskResults> LoadEndFrame(size_t id);
    std::unique_ptr<TaskResults> LoadTaskResultsLastNFrames(size_t id, size_t nFrames);
    std::unique_ptr<TaskResults> LoadTaskResults(size_t id);

    virtual void ProvideDatasetNames(std::vector<std::string> names) = 0;
    void SaveFile(std::string fileName, std::vector<float_p> data);
    void SaveFile(std::string fileName, std::vector<std::vector<float_p>> data);
    void SaveFile(std::string fileName, std::vector<complex_p> data);
    void SaveFile(std::string fileName, std::vector<std::vector<complex_p>> data);
    // Another save

    virtual const std::set<size_t>& UsedIds() = 0;

protected:
    //virtual void DataOpen() = 0;
    std::string rootFolder;
    std::string dataStoreBasePath;
    size_t idealFileSize = 1024*1024*1024;

    bool isFileReady = false;
    bool isReadable = false;
    bool isWritable = false;

    virtual std::unique_ptr<TaskResults> LoadResult(size_t id, bool lastFrameOnly = false) = 0;

    virtual void StoreData(std::unique_ptr<TaskResults> const& results) = 0;
    virtual void StoreDataSimple(std::unique_ptr<TaskResults> const& results) = 0;
    virtual void LoadListOfStoredDataEvents() = 0;

    Settings::SaveSettings saveStatus;
private:
    
    void CreateFolder(std::string folder);
};


#endif /* DataStore_hpp */
