//
//  Settings.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef Settings_hpp
#define Settings_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <boost/property_tree/ptree_fwd.hpp>

namespace pt = boost::property_tree;

using namespace std;

class Settings
{
public:
    Settings(int argc, char* argv[] );
    ~Settings();
    
    enum Status { invalid, firstRun, subsequentRun };
    Status status = Status::invalid;
    
    template<class T> T get(string value) const;
    
protected:
    void SetupOutputFolder();
    
    pt::ptree* tree;
    std::string basePath;
    
    string inputPathStr;
    string outputPathStr;
private:
    
};
#endif /* Settings_hpp */
