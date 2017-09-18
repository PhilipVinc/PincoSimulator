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

using namespace std;

const unsigned char magic8Byte[8] = {0x89, 'P', 'N', 'C', 0x0d, 0x0a, 0x1a, 0x0a};
const unsigned char fileVersion = 1;

ChunkRegister::ChunkRegister(const Settings* _settings, std::string _path)
{
    settings = _settings;
    registerFilePath = _path;
    OpenRegister();
}

ChunkRegister::~ChunkRegister()
{
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
    if (settings->status == Settings::Status::firstRun)
    {
        registerFile = fopen(registerFilePath.c_str(), "wb");
        registerInitialized = false;
    }
    else
    {
        registerFile = fopen(registerFilePath.c_str(), "wb+");
        registerInitialized = true;
        
        if (registerFile)
        {
            registerEntry buffer;
            while (fread(&buffer, sizeof(registerEntry), 1, registerFile))
            {
                entries.push_back(buffer);
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
    fwrite(dimBuffer, sizeof(size_t), jk, registerFile);
    
    // 5) WRite the Extra Data size for each entry
    auto offsetTraj = results->SerializingExtraDataOffset();
    fwrite(&offsetTraj, 1, sizeof(offsetTraj), registerFile);

    // 6) Write the offset to the data; now it is just the current position
    auto dataoffset = ftell(registerFile);
    fwrite(&dataoffset, 1, sizeof(dataoffset), registerFile);
    fflush(registerFile);
    
    //                  id +chunk_id+chunkoffset+continuation+ additionalinfo
    sizeOfTrajEntry = 4*sizeof(size_t)  +   offsetTraj;
    
    trajBuffer = new size_t[sizeOfTrajEntry/sizeof(size_t)];
    
    registerInitialized = true;
}

void ChunkRegister::RegisterStoredData(TaskResults* results,
                                       size_t chunkId,
                                       size_t chunkOffset,
                                       Settings::SaveSettings saveType)
{
    if (!registerInitialized)
        InitializeRegisterHeader(results);
    
    trajBuffer[0] = results->GetId();
    trajBuffer[1] = chunkId;
    trajBuffer[2] = chunkOffset;
    trajBuffer[3] = 0;
    memcpy(&trajBuffer[4], results->SerializeExtraData(), results->SerializingExtraDataOffset());
    fwrite(trajBuffer, 1, sizeOfTrajEntry, registerFile);
    
    if (saveType == Settings::SaveSettings::appendIdFiles)
    {
        
    }
}

void AddContinuationToRegister(size_t traj_id, size_t chunk_id,
                               size_t frame0, size_t frameEnd)
{
    
}

