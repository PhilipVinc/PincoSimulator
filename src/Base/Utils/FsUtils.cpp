//
//  FsUtils.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "FsUtils.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;

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

void CheckCreateFolder(string folderName)
{
    struct stat sb;
    if (!(stat(folderName.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)))
    {
        mkpathAll(folderName.c_str());
    }
}

