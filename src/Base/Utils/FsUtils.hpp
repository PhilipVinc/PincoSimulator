//
//  FsUtils.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef FsUtils_hpp
#define FsUtils_hpp

#include <stdio.h>
#include <string>
#include <vector>

#include <sys/types.h>

std::vector<std::string> findAllFolders(std::string path);
std::vector<std::string> findFoldersContainingString(std::string path, std::string pattern);

int mkpathAll(const char* _file_path);
void CheckCreateFolder(std::string folderName);

#endif /* FsUtils_hpp */
