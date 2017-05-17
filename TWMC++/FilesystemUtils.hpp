//
//  FilesystemUtils.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef FilesystemUtils_hpp
#define FilesystemUtils_hpp

#include <stdio.h>
#include <string>
#include <vector>

#include <sys/types.h>

unsigned int file_exist(const std::string fileName);
int mkpathAll(const char* _file_path);
int mkpath(char* file_path, mode_t mode);
std::vector<std::string> findAllFolders(std::string path);
std::vector<std::string> findFoldersContainingString(std::string path, std::string pattern);
void CheckCreateFolder(std::string folderName);
void CopyFileToFolder(std::string inputFile, std::string outputFolder);

#endif /* FilesystemUtils_hpp */
