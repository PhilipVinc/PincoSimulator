//
//  FilesystemUtils.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "FilesystemUtils.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;


const string paramsFileName = "_sim.ini";
const string folderSeparator = "/";
const string trajectoryFolderBaseName = "trajectories";
const string trajectoryNames = "traj";
const string trajectoryExtension = ".bin";


unsigned int file_exist(const string fileName)
{
    struct stat st;
    stat(fileName.c_str(), &st);
    bool isDir = S_ISDIR(st.st_mode);
    if (isDir)
        return 2;
    else
    {
        std::ifstream infile(fileName);
        return infile.good();
    }
}

int mkpathAll(const char* _file_path) {
    char file_path[1500];
    strcpy(file_path,_file_path);
    
    //assert(file_path && *file_path);
    char* p;
    for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
        *p='\0';
        if (mkdir(file_path, 0777)==-1) {
            if (errno!=EEXIST) { *p='/'; return -1; }
        }
        *p='/';
    }
    return 0;
}

int mkpath(char* file_path, mode_t mode) {
    //assert(file_path && *file_path);
    char* p;
    for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
        *p='\0';
        if (mkdir(file_path, mode)==-1) {
            if (errno!=EEXIST) { *p='/'; return -1; }
        }
        *p='/';
    }
    return 0;
}


std::vector<string> findAllFolders(string path)
{
    std::vector<string> result = std::vector<string>(0);
    
    const char* PATH = path.c_str();
    
    DIR *dir = opendir(PATH);
    struct dirent *entry = readdir(dir);
    
    while (entry != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            printf("%s\n", entry->d_name);
            result.push_back(entry->d_name);
        }
        
        entry = readdir(dir);
    }
    closedir(dir);
    return result;
}

std::vector<string> findFoldersContainingString(string path, string pattern)
{
    std::vector<string> allFolders = findAllFolders(path);
    for (int i=0; i< allFolders.size(); i++)
    {
        if (allFolders[i].find(pattern) == std::string::npos)
        {
            allFolders.erase(allFolders.begin()+i);
            i--;
        }
    }
    return allFolders;
}

void CheckCreateFolder(string folderName)
{
    struct stat sb;
    if (!(stat(folderName.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)))
    {
        mkpathAll(folderName.c_str());
    }
}


void CopyFileToFolder(string inputFile, string outputFolder)
{
    std::ifstream  src(inputFile, std::ios::binary);
    std::ofstream  dst(outputFolder+folderSeparator+paramsFileName,   std::ios::binary);
    
    dst << src.rdbuf();
    src.close();
    dst.close();
}
