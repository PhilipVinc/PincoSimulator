//
//  ChunkRegister.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ChunkRegister.hpp"
#include <iostream>
#include "TaskResults.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <boost/filesystem.hpp>
#pragma clang pop

using namespace std;
namespace fs = boost::filesystem;


using namespace std;

const unsigned char magic8Byte[8] = {0x89, 'P', 'N', 'C', 0x0d, 0x0a, 0x1a, 0x0a};
const unsigned char fileVersion = 1;

ChunkRegister::ChunkRegister(std::string _path)
{
    cout << "Creating ChunkRegister" << endl;
    registerFilePath = _path;
    OpenRegister();
}

ChunkRegister::~ChunkRegister()
{
    cout << "Deleting ChunkRegister" << endl;
    if (registerFile)
    {
        fclose(registerFile);
    }
    
    if (registerInitialized)
    {
        delete[] trajBuffer;
    }
}

void ChunkRegister::OpenRegister()
{
    // Let's create the register
    if (!fs::exists(registerFilePath))
    {
        registerFile = fopen(registerFilePath.c_str(), "wb");
        newRun = true;
        registerInitialized = false;
    }
    else
    {
        registerFile = fopen(registerFilePath.c_str(), "rb+");
        registerInitialized = true;
        
        
        if (registerFile)
        {
            ReadRegisterHeader();
            
            trajBuffer = new size_t[sizeOfTrajEntry/sizeof(size_t)];

            while(fread(trajBuffer, 1, sizeOfTrajEntry, registerFile)==sizeOfTrajEntry)
            {
                registerEntry* entry = new registerEntry;
                entry->traj_id = trajBuffer[0];
                entry->chunk_id = trajBuffer[1];
                entry->chunk_offset = trajBuffer[2];
                entry->continuation_offset = trajBuffer[3];
                entry->additionalData = new size_t[trajAdditionalDataSize/sizeof(size_t)];
                memcpy(entry->additionalData, &trajBuffer[4], trajAdditionalDataSize);
                entries.push_back(entry);
                storedEntries++;
            }
        }
        else
        {
            cerr << "Error Reading Register File" << endl;
        }
    }
}

void ChunkRegister::InitializeRegisterHeader(TaskResults* results)
{
    // -- Initialize the file
    // 1) write the magic 8 bytes
    fwrite(magic8Byte, 1, sizeof(magic8Byte), registerFile);
    // 2) Write the file version number
    fwrite(&fileVersion, 1, sizeof(fileVersion), registerFile);
    
    // 3) Write the dataset names
    auto datasetNames = results->NamesOfDatasets();
    fputc(datasetNames.size(), registerFile);
    for (auto name : datasetNames)
    {
        fputc(name.size(), registerFile);
        fwrite(name.c_str(), 1, name.size(), registerFile);
    }
    
    // 4) Write the datast dimensions
    auto dimData = results->DataSetsDimensionData();
    size_t ji = 0; size_t jk = 0;
    size_t* dimBuffer = new size_t[5*results->NumberOfDataSets()];
    
    for (int i =0; i!=datasetNames.size(); i++)
    {
        auto D = results->DataSetDimension(i);
        dimBuffer[ji] = D;
        ji++;
        for (size_t j = 0; j!=D; j++)
        {
            dimBuffer[ji] = dimData[jk];
            jk++; ji++;
        }
    }
    fwrite(dimBuffer, sizeof(size_t), ji, registerFile);
    delete[] dimBuffer;
    
    // 5) WRite the Extra Data size for each entry
    long int offsetTraj = results->SerializingExtraDataOffset();
    fwrite(&offsetTraj, 1, sizeof(offsetTraj), registerFile);
    

    // 6) Write the offset to the data; now it is just the current position
    additionalDataLengthOffset = ftell(registerFile);
    fwrite(&additionalDataLengthOffset, 1, sizeof(additionalDataLengthOffset), registerFile);
    
    // Write data
    additionalDataBegin = ftell(registerFile);
    
    // End writing additional data
    trajDataBegin = ftell(registerFile);
    fseek(registerFile, additionalDataLengthOffset, SEEK_SET);
    fwrite(&trajDataBegin, 1, sizeof(trajDataBegin), registerFile);
    fflush(registerFile);
    
    fseek(registerFile, trajDataBegin, SEEK_SET);
    
    // id +chunk_id+chunkoffset+continuation+ additionalinfo
    sizeOfTrajEntry = 4*sizeof(size_t)  +   offsetTraj;
    
    trajBuffer = new size_t[sizeOfTrajEntry/sizeof(size_t)];
    
    
    additionalDataBegin = additionalDataLengthOffset + sizeof(offsetTraj);
    trajAdditionalDataSize = offsetTraj;
    
    registerInitialized = true;
}

void ChunkRegister::ReadRegisterHeader()
{
    unsigned char tmpBfr[8];
    unsigned char fileVersion;
    // Check magic byte & Version number
    fread(&tmpBfr, 1, sizeof(magic8Byte), registerFile);
    fread(&fileVersion, 1, sizeof(fileVersion), registerFile);
    
    // Read Dataset Names
    int nOfVariables = fgetc(registerFile);
    vector<string> variableNames(nOfVariables);
    for (int i = 0; i != nOfVariables; i++)
    {
        variableNames[i].resize(fgetc(registerFile));
        fread(&(variableNames[i][0]), 1, variableNames[i].size(), registerFile);
    }
    
    // Read Dataset dimensions
    size_t* dimBuffer = new size_t[5*nOfVariables]; size_t ik=0;
    for (int i = 0; i != nOfVariables; i++)
    {
        fread(&dimBuffer[ik], 1, sizeof(size_t), registerFile);
        
        if (dimBuffer[ik] != 0)
        {
            fread(&dimBuffer[ik+1], 1, dimBuffer[ik]*sizeof(size_t), registerFile);
        }
        ik += dimBuffer[ik]+1;
    }
    
    fread(&trajAdditionalDataSize, 1, sizeof(trajAdditionalDataSize), registerFile);
    fread(&trajDataBegin, 1, sizeof(trajDataBegin), registerFile);
    
    
    sizeOfTrajEntry = 4*sizeof(size_t)+trajAdditionalDataSize;
    
    GoToTrajectoryDataBegin();
}

void ChunkRegister::GoToTrajectoryDataBegin()
{
    fseek(registerFile, trajDataBegin, SEEK_SET);
}

void ChunkRegister::GoToCurrentWritePosition()
{
    fseek(registerFile, storedEntries*sizeOfTrajEntry + trajDataBegin, SEEK_SET);
}

void ChunkRegister::RegisterStoredData(TaskResults* results,
                                       size_t chunkId,
                                       size_t chunkOffset,
                                       Settings::SaveSettings saveType)
{
    // If this is the first trajectory to be saved in the register, we have to
    // initialize it
    if (!registerInitialized)
        InitializeRegisterHeader(results);
    
    trajBuffer[0] = results->GetId();
    trajBuffer[1] = chunkId;
    trajBuffer[2] = chunkOffset;
    trajBuffer[3] = 0;
    memcpy(&trajBuffer[4], results->SerializeExtraData(), results->SerializingExtraDataOffset());
    
    // If this is an append, then we will have to correct the old data with a 
    // pointer to actual data
    if (saveType == Settings::SaveSettings::appendIdFiles)
    {
        auto continuationOffset = ftell(registerFile);
    }
    ftell(registerFile);
    fwrite(trajBuffer, 1, sizeOfTrajEntry, registerFile);
    fflush(registerFile);
    storedEntries++;
}


size_t ChunkRegister::GetNumberOfSavedTasks()
{
    return storedEntries;
}


void AddContinuationToRegister(size_t traj_id, size_t chunk_id,
                               size_t frame0, size_t frameEnd)
{
    
}

std::set<size_t> ChunkRegister::GetUsedChunkIds()
{
    for (size_t i=0; i != storedEntries; i++)
    {
        chunkIds.insert(entries[i]->chunk_id);
    }
    return chunkIds;
}
