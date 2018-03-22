//
//  ChunkRegister.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 15/09/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "ChunkRegister.hpp"

#include "Base/TaskResults.hpp"
#include "Libraries/FilesystemLibrary.h"

#include <algorithm>
#include <memory>
#include <iostream>
#include <cstring> // For memcpy bo gcc


using namespace std;

const unsigned char magic8Byte[8] = {0x89, 'P', 'N', 'C', 0x0d, 0x0a, 0x1a, 0x0a};
const unsigned char fileVersion = 1;

ChunkRegister::ChunkRegister(std::string _path)
{
    cout << "Instantiating ChunkRegister Class" << endl;
    registerFilePath = _path;

    if (OpenRegisterFile()) {
        ReadRegisterEntries();
    } else {
        CreateNewRegisterFile();
    }
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

        for (auto entryPtr : entries) {
            delete[] entryPtr->additionalData;
            delete entryPtr;
        }
    }
}

bool ChunkRegister::ReadRegisterEntries()
{
    if (registerInitialized)
    {
	    // Go back to the beginning
	    GoToTrajectoryDataBegin();
	    entries.reserve(storedEntries);

	    trajBuffer = new size_t[sizeOfTrajEntry/sizeof(size_t)];

        while(fread(trajBuffer, 1, sizeOfTrajEntry, registerFile)==sizeOfTrajEntry)
        {
            RegisterEntry* entry = new RegisterEntry;
            entry->traj_id = trajBuffer[0];
            entry->chunk_id = trajBuffer[1];
            entry->chunk_offset = trajBuffer[2];
            entry->continuation_offset = trajBuffer[3];
            entry->additionalData = new char[trajAdditionalDataSize];
            std::memcpy(entry->additionalData, &trajBuffer[4], trajAdditionalDataSize);
	        entry->registerWritePosition = ftell(registerFile)-sizeOfTrajEntry;
	        entries.push_back(entry);
            trajOffsets.insert(std::pair<size_t, size_t>(trajBuffer[0], entries.size()-1));
            trajIds.insert(trajBuffer[0]);
            storedEntries++;
        }
	    registerDataRead = true;

	    return true;
    }
    else
    {
        cerr << "Error Reading Register File" << endl;
	    return false;
    }
}

bool ChunkRegister::CreateNewRegisterFile()
{
    cout << "Creating new register file at path: " << registerFilePath << endl;
    registerFile = fopen(registerFilePath.c_str(), "wb");
    newRun = true;
    registerInitialized = false;

    return true;
}

bool ChunkRegister::OpenRegisterFile()
{
    if (!filesystem::exists(registerFilePath))
        return false;

    cout << "Loading Register file: " << registerFilePath << endl;
    registerFile = fopen(registerFilePath.c_str(), "rb+");
    size_t version = CheckRegisterVersion();

    // File is invalid
    if (version == 0) {
        fclose(registerFile);
        return false;
    }

    ReadRegisterHeader();


    return true;
}

size_t ChunkRegister::CheckRegisterVersion()
{
    size_t oldPos = ftell(registerFile);
    fseek(registerFile, 0, SEEK_SET);

    unsigned char tmpBfr[8];
    unsigned char fileVersion;
    // Check magic byte & Version number
    fread(&tmpBfr, 1, sizeof(magic8Byte), registerFile);
    fread(&fileVersion, 1, sizeof(fileVersion), registerFile);

    bool ok = true;
    for (int i = 0; i<8 ; i++)
    {
        if (tmpBfr[i] != magic8Byte[i])
            ok=false;
    }
    if (!ok)
    {
        cerr << "ERROR LOADING Register FILE. Creating new Register" << endl;
        return 0;
    }

    fseek(registerFile, oldPos, SEEK_SET);

    return fileVersion;
}

void ChunkRegister::InitializeRegisterHeader(std::unique_ptr<TaskResults> const& results)
{
    // -- Initialize the file
    // 1) write the magic 8 bytes
    fwrite(magic8Byte, 1, sizeof(magic8Byte), registerFile);
    // 2) Write the file version number
    fwrite(&fileVersion, 1, sizeof(fileVersion), registerFile);
    
    // 3) Write the dataset names
    datasetNames = results->NamesOfDatasets();
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
        size_t D = results->DataSetDimension(i);
        dimensionalityData.push_back(std::vector<size_t>());
        dimBuffer[ji] = D;
        ji++;
        for (size_t j = 0; j!=D; j++)
        {
            dimensionalityData.back().push_back(dimData[jk]);
            dimBuffer[ji] = dimData[jk];
            jk++; ji++;
        }
    }
    fwrite(dimBuffer, sizeof(size_t), ji, registerFile);
    delete[] dimBuffer;

    for (int i=0; i!=datasetNames.size(); i++)
    {
        fputc(results->DataSetDataType(i), registerFile);
    }
    
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

	// Estimate number of entries:
	fseek(registerFile, 0, SEEK_END);
	auto trajEnd = ftell(registerFile);
	storedEntries = (trajEnd - trajDataBegin)/sizeOfTrajEntry;
	GoToCurrentWritePosition();

	registerInitialized = true;
}

bool ChunkRegister::ReadRegisterHeader()
{
	if (registerFile) {
		unsigned char tmpBfr[8];
		unsigned char fileVersion;
		// Check magic byte & Version number
		fread(&tmpBfr, 1, sizeof(magic8Byte), registerFile);
		fread(&fileVersion, 1, sizeof(fileVersion), registerFile);

		bool ok = true;
		for (int i = 0; i < 8; i++) {
			if (tmpBfr[i] != magic8Byte[i])
				ok = false;
		}
		if (!ok) {
			cerr << "ERROR LOADING Register FILE. ABORTING" << endl;
			return false;
		}

		// Read Dataset Names
		int nOfVariables = fgetc(registerFile);
		vector<string> variableNames(nOfVariables);
		for (int i = 0; i != nOfVariables; i++) {
			variableNames[i].resize(fgetc(registerFile));
			fread(&(variableNames[i][0]), 1, variableNames[i].size(), registerFile);
		}
        datasetNames = variableNames;

		// Read Dataset dimensions
		size_t *dimBuffer = new size_t[5 * nOfVariables];
		size_t ik = 0;
		for (int i = 0; i != nOfVariables; i++) {
			fread(&dimBuffer[ik], 1, sizeof(size_t), registerFile);

			if (dimBuffer[ik] != 0) {
				fread(&dimBuffer[ik + 1], 1, dimBuffer[ik] * sizeof(size_t), registerFile);
			}
            dimensionalityData.push_back(std::vector<size_t>(&dimBuffer[ik+1], &dimBuffer[ik+1+dimBuffer[ik]]));
			ik += dimBuffer[ik] + 1;
		}
        delete[] dimBuffer;

		// Read type of variables
		int* varTypes = new int[nOfVariables];
		for (int i = 0; i != nOfVariables; i++) {
			varTypes[i] = fgetc(registerFile);
			cout << "var[" << i << "] has typeId = " << varTypes[i] << endl;
		}
        delete[] varTypes;
		// Read additionalDataSize and location trackers
		fread(&trajAdditionalDataSize, 1, sizeof(trajAdditionalDataSize), registerFile);
		fread(&trajDataBegin, 1, sizeof(trajDataBegin), registerFile);


		sizeOfTrajEntry = 4 * sizeof(size_t) + trajAdditionalDataSize;

		GoToTrajectoryDataBegin();
		registerInitialized = true;

		return true;
	} else{
		throw std::string("Register file was not open!");
	}
}

inline void ChunkRegister::GoToTrajectoryDataBegin()
{
    fseek(registerFile, trajDataBegin, SEEK_SET);
}

inline void ChunkRegister::GoToCurrentWritePosition()
{
    fseek(registerFile, storedEntries*sizeOfTrajEntry + trajDataBegin, SEEK_SET);
}

void ChunkRegister::RegisterStoredData(std::unique_ptr<TaskResults> const& results,
                                       size_t chunkId,
                                       size_t chunkOffset,
                                       Settings::SaveSettings saveType)
{
    // If this is the first trajectory to be saved in the register, we have to
    // initialize it
    if (!registerInitialized) {
	    InitializeRegisterHeader(results);
    }

    if (saveType == Settings::SaveSettings::unspecified) {
        if (trajIds.count(results->GetId()) == 0) {
            saveType = Settings::SaveSettings::saveIdFiles;
        } else {
            saveType = Settings::SaveSettings::appendIdFiles;
        }
    }

    // Populate the 4+extra fields we store in the data
    trajBuffer[0] = results->GetId();
    trajBuffer[1] = chunkId;
    trajBuffer[2] = chunkOffset;
    trajBuffer[3] = 0;
    std:memcpy(&trajBuffer[4], results->SerializeExtraData(), results->SerializingExtraDataOffset());

    auto continuationOffset = ftell(registerFile);
    fseek(registerFile, 0, SEEK_END);
    // write the data to the register file
    fwrite(trajBuffer, 1, sizeOfTrajEntry, registerFile);


    // If this is an append, then we will have to correct the old data with a 
    // pointer to actual data
    if (saveType == Settings::SaveSettings::appendIdFiles) {
	    auto entry = GetEntryById(results->GetId());

	    fseek(registerFile, entry->registerWritePosition + 3*sizeof(size_t) , SEEK_SET);
	    fwrite(&continuationOffset, 1, sizeof(size_t), registerFile);
	    fseek(registerFile, 0, SEEK_END);

        // Update the old entry register
        auto prevEntries = trajOffsets.equal_range(trajBuffer[0]);
        for (auto it = prevEntries.first; it != prevEntries.second; it++) {
            if (entries[it->second]->continuation_offset == 0) {
                entries[it->second]->continuation_offset = continuationOffset;
            }
        }
    }
    fflush(registerFile);

    // Prepare the entry to be stored here
    {
        RegisterEntry* entry = new RegisterEntry;
        entry->traj_id = trajBuffer[0];
        entry->chunk_id = trajBuffer[1];
        entry->chunk_offset = trajBuffer[2];
        entry->continuation_offset = trajBuffer[3];
        entry->additionalData = new char[trajAdditionalDataSize];
        std::memcpy(entry->additionalData, &trajBuffer[4], trajAdditionalDataSize);
        entry->registerWritePosition = continuationOffset;
        entries.push_back(entry);
        trajOffsets.insert(std::pair<size_t, size_t>(trajBuffer[0], entries.size()));
        trajIds.insert(trajBuffer[0]);
    }

    //ftell(registerFile);
    storedEntries++;
}

inline size_t ChunkRegister::GetNumberOfSavedTasks()
{
    return storedEntries;
}

inline size_t ChunkRegister::GetNumberOfSavedTrajectories()
{
    return trajIds.size();
}

std::set<size_t> ChunkRegister::GetUsedChunkIds()
{
	if(!registerDataRead)
		ReadRegisterEntries();

	for (size_t i=0; i != storedEntries; i++)
    {
        chunkIds.insert(entries[i]->chunk_id);
    }
    return chunkIds;
}

const std::set<size_t>& ChunkRegister::GetSavedTasksIds() {
	if(!registerDataRead)
		ReadRegisterEntries();

	return trajIds;
}

inline ChunkRegister::RegisterEntry* ChunkRegister::GetEntryByPosition(size_t index)
{
	return entries[index];
}

ChunkRegister::RegisterEntry* ChunkRegister::GetEntryById(size_t id)
{
	/*auto check = [id](const RegisterEntry* entry) {
		return entry->traj_id == id;
	};
	std::vector<RegisterEntry*>::iterator regEntry = std::find_if(std::begin(entries), std::end(entries), check);
	return *regEntry;*/
    auto prevEntries = trajOffsets.equal_range(id);
    size_t maxIndex = 0;
    for (auto it = prevEntries.first; it != prevEntries.second; it++) {
        if ( it->second > maxIndex ) {
            maxIndex = it->second;
        }
    }
    return GetEntryByPosition(maxIndex);
}
