/**
    \class DataStore
    \ingroup FileFormats

    \brief Base Virtual class for DataStore formats

    This class is a base virtual class from which all FileFormat Datastores
    must inherit. It enforces a common API to which all fileformats must
    conform for saving and loading trajectory data. It also provides a few
    utility methods to save floating point data as text to a file.

    \author $Author: Filippo Vicentin $
    \date $Date: 22/09/17 $
    \copyright © 2017 Filippo Vicentini. All rights reserved.
*/

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
    /// Create the Datastore, initialising it's folder
    DataStore(const Settings* settings, std::string folderName);
    virtual ~ DataStore() {};

    /// Save one TaskResults to file, either creating a new entry or appending
    bool SaveTaskResults(std::unique_ptr<TaskResults> const& task);

    /// Returns a TaskResults with the last frame of entry id
    std::unique_ptr<TaskResults> LoadEndFrame(size_t id);
    /// Returns a TaskResults with last nFrames of entry id
    std::unique_ptr<TaskResults> LoadTaskResultsLastNFrames(size_t id, size_t nFrames);
    /// Returns a TaskResults of entry id
    std::unique_ptr<TaskResults> LoadTaskResults(size_t id);

    /// Provide the names of the dataset to be saved. @Deprecate
    virtual void ProvideDatasetNames(std::vector<std::string> names) = 0;

    /// Save a vector as textual data to file fileName
    void SaveFile(std::string fileName, std::vector<float_p> data);
    void SaveFile(std::string fileName, std::vector<complex_p> data);
    /// Save a matrix (vector of vectors) as textual data to file fileName
    void SaveFile(std::string fileName, std::vector<std::vector<float_p>> data);
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
