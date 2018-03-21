//
//  DataStore.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "DataStore.hpp"

#include "ChunkRegister.hpp"
#include "ChunkFileSet.hpp"
#include "Libraries/FilesystemLibrary.h"
#include "../Utils/FsUtils.hpp"
#include "../Settings.hpp"
#include "../TaskResults.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;


DataStore::DataStore(const Settings* settings, string folderName)
{
    rootFolder = folderName;
    filesystem::path rootPath = folderName;

    dataStoreBasePath = (rootPath  / "data" ).string();

    CreateFolder(dataStoreBasePath);
    saveStatus = settings->saveStatus;
}


bool DataStore::SaveTaskResults(std::unique_ptr<TaskResults> const& task)
{
    StoreData(task);
    return true;
}

std::unique_ptr<TaskResults> DataStore::LoadEndFrame(size_t id) {
    return LoadLastResultFrame(id);
}


void DataStore::CreateFolder(string folder)
{
    filesystem::path dir(folder);
    if (filesystem::is_directory(dir))
        std::cout << "Folder " << folder << " alredy exists." << "\n";
    else if (filesystem::create_directory(dir))
        std::cout << "Created folder: " <<  folder << endl;
    else
        std::cerr << "Error creating folder: " << folder << "\n";
}

void DataStore::SaveFile(string fileName, vector<float_p> data)
{
    filesystem::path pp = dataStoreBasePath;
    pp = pp / fileName;

    //FILE* f = fopen(pp.string().c_str(), "w+");
    ofstream f;
    f.open(pp.string(), ofstream::out);
    f << "%\tformat:real\tcols:1\trows:" << data.size() << endl;
    for (float_p i : data) {
        f << i << endl;
    }
    f.close();
    //fwrite(data, sizeof(data), length, f);
    //fclose(f);
}

void DataStore::SaveFile(string fileName, vector<vector<float_p>> data)
{
    filesystem::path pp = dataStoreBasePath;
    pp = pp / fileName;

    ofstream f;
    f.open(pp.string(), ofstream::out);
    f << "%\tformat:real\tcols:x\trows:" << data.size() << endl;
    size_t nxy = data[0].size();
    for(size_t i =0; i< data.size(); i++)
    {
        for (size_t j = 0; j < nxy; j++ ) {
            f << data[i][j] << "\t";
        }
        f << endl;
    }
    f.close();
}

void DataStore::SaveFile(string fileName, vector<complex_p> data)
{
    filesystem::path pp = dataStoreBasePath;
    pp = pp / fileName;

    ofstream f;
    f.open(pp.string(), ofstream::out);
    f << "%\tformat:complex\tcols:1\trows:" << data.size() << endl;
    for (complex_p i : data) {
        f << real(i) << "\t" << imag(i) << endl;
    }
    f.close();
}

void DataStore::SaveFile(string fileName, vector<vector<complex_p>> data)
{
    filesystem::path pp = dataStoreBasePath;
    pp = pp / fileName;

    ofstream f;
    f.open(pp.string(), ofstream::out);
    f << "%\tformat:complex\tcols:x\trows:" << data.size() << endl;
    size_t nxy = data[0].size();
    for(size_t i =0; i< data.size(); i++)
    {
        for (size_t j = 0; j < nxy; j++ ) {
            f << real(data[i][j]) << "\t" << imag(data[i][j]) << "\t";
        }
        f << endl;
    }
    f.close();
}
