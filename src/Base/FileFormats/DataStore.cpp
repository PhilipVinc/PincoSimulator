//
//  DataStore.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "DataStore.hpp"
#include "Settings.hpp"
#include "FsUtils.hpp"

#include "TaskResults.hpp"
#include "ChunkRegister.hpp"
#include "ChunkFileSet.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <boost/filesystem.hpp>
#pragma clang pop


using namespace std;
namespace fs = boost::filesystem;


DataStore::DataStore(const Settings* settings, string folderName)
{
    rootFolder = folderName;
    fs::path rootPath = folderName;
    
    //auto trajectoryFoldersN = findFoldersContainingString(rootFolder,
     //                                                     settings->trajectoryFolderBaseName).size()+1;
   
    dataStoreBasePath = (rootPath  / "data" ).string();
    
    
    CreateFolder(dataStoreBasePath);
    saveStatus = settings->saveStatus;
}


bool DataStore::SaveTaskResults(TaskResults* task)
{
    StoreDataSimple(task);
    return true;
}

void DataStore::CreateFolder(string folder)
{
    boost::filesystem::path dir(folder);
    if (boost::filesystem::create_directory(dir))
        std::cout << "Success creating Folder" << "\n";
    else
        std::cerr << "Error creating Folder" << "\n";
}

void DataStore::SaveFile(string fileName, float_p* data, size_t length)
{
    fs::path pp = dataStoreBasePath;
    pp = pp / fileName;
 
    
    //FILE* f = fopen(pp.string().c_str(), "w+");
    ofstream f;
    f.open(pp.string(), ofstream::out);
    for(int i =0; i< length; i++)
    {
        f << data[i] << endl;
    }
    f.close();
    //fwrite(data, sizeof(data), length, f);
    //fclose(f);
}
