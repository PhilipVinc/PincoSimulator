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
#include <vector>

#include "PincoFormatDataStore.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <boost/filesystem.hpp>
#pragma clang pop

using namespace std;
namespace fs = boost::filesystem;

DataSaver::DataSaver(const Settings* settings)
{
    saveStatus = settings->saveStatus;
    dataStore = new PincoFormatDataStore(settings, settings->GetRootFolder());
}

DataSaver::~DataSaver()
{
    delete dataStore;
}

void DataSaver::SaveData(TaskResults *results)
{
    //cout << "Saving "<< results->GetId() << endl;
    dataStore->SaveTaskResults(results);
}

void DataSaver::ProvideDatasetNames(vector<string> names)
{
    dataStore->ProvideDatasetNames(names);
}

void DataSaver::SaveFile(string fileName, vector<float_p> data)
{
    dataStore->SaveFile(fileName, data);
}

void DataSaver::SaveFile(string fileName, vector<vector<float_p>> data)
{
    dataStore->SaveFile(fileName, data);
}

