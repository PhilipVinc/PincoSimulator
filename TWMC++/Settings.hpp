//
//  Settings.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef Settings_hpp
#define Settings_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <boost/property_tree/ptree_fwd.hpp>

namespace pt = boost::property_tree;
using namespace std;

/*! 
    @brief Class containing all settings relative to a single run of the program.
 
    @discussion This class is created by passing to it the command line arguments that were used to launch the program, and It will load the relative ini file, overriding options with those found in the command line.
                It is also reposnsible for loading data types from the ini file, and some custom data types from files indicated in the ini file.
 */
class Settings
{
public:
    Settings(int argc, char* argv[] );
    ~Settings();
    
    template<class T> T get(string value) const;
    unsigned int GlobalSeed() const;
    string GetOutputFolder() const;
    string GetRootFolder() const;
    bool IsOutputFolderSet() const;

    /// @brief Enum indicating what type of execution this is
    /// @discussion invalid - indicates that the data provided is invalid and execution wil soon abort;
    ///             firstRun - indicates that there was no data prior to this execution;
    ///             subsequentRun - indicates that we are resuming a previous run;
    enum Status { invalid, firstRun, subsequentRun, appendRun };
    enum SaveSettings {dontSave, saveSingleFile, saveIdFiles, appendIdFiles};
    
    Status status = Status::invalid;
    SaveSettings saveStatus = SaveSettings::saveIdFiles;

protected:    
    pt::ptree* tree;
    string basePath;
    
    string inputPathStr;
    string outputPathStr;
    string inputParentPathStr;
    
    const string paramsFileName = "_sim.ini";
    const string trajectoryFolderBaseName = "trajectories";
    
    bool outputFolderIsSet = false;
private:
    
};
#endif /* Settings_hpp */
