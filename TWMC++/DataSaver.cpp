//
//  DataSaver.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 25/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "DataSaver.hpp"
#include "Settings.hpp"
#include "FsUtils.hpp"
#include "TaskResults.hpp"
#include "ChunkRegister.hpp"
#include "ChunkFileSet.hpp"

#include <string> 
#include <iostream>
#include <stdio.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <boost/filesystem.hpp>
#pragma clang pop

using namespace std;
namespace fs = boost::filesystem;

DataSaver::DataSaver(const Settings* settings)
{
    fs::path rootPath;
    if (settings->IsOutputFolderSet())
        saveBasePath = settings->GetOutputFolder();
    else
    {
        rootFolder = settings->GetRootFolder();
        rootPath = rootFolder;
        
        auto trajectoryFoldersN = findFoldersContainingString(rootFolder, trajectoryFolderBaseName).size()+1;
        saveBasePath = (rootPath  / (trajectoryFolderBaseName + to_string(trajectoryFoldersN))).string();

    }
    
    CreateFolder(saveBasePath);
    saveStatus = settings->saveStatus;
    
    fileNamePrepend = saveBasePath + "/";
    fileNameAppend = ".bin";
    
    chunkRegisterFileName = saveBasePath + "/_register.cnk";
    chunkRegister = new ChunkRegister(settings, chunkRegisterFileName);
}

DataSaver::~DataSaver()
{
    
}

void DataSaver::SaveData(TaskResults *results)
{
    cout << "Saving "<< results->GetId() << endl;
    size_t N = results->NumberOfDataSets();
    
    std::vector<size_t> offsets(N);
    
    if (chunkCache==NULL)
    {
        chunkCache=new ChunkFileSet(fileNamePrepend, results->NamesOfDatasets(),
                                    currentChunkId);
    }
    
    size_t offset = chunkCache->WriteToChunk(results);
    
    chunkRegister->RegisterStoredData(results, currentChunkId, offset, saveStatus);
    
    if (chunkCache->IsChunkBig())
    {
        delete chunkCache;
        chunkCache = NULL;
        currentChunkId++;
    }
}

void DataSaver::CloseFile(FILE* file)
{
    switch (saveStatus)
    {
        case Settings::SaveSettings::saveIdFiles:
            fclose(file);
            break;
        case Settings::SaveSettings::appendIdFiles:
            fclose(file);
            break;
        default:
            break;
    }

}

void DataSaver::CreateFolder(string folder)
{
    boost::filesystem::path dir(folder);
    if (boost::filesystem::create_directory(dir))
        std::cout << "Success" << "\n";
    else
        std::cerr << "Error" << "\n";
}

