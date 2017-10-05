//
//  TaskResultsFactory.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "TaskResultsFactory.hpp"
#include "TaskResults.hpp"
#include "Settings.hpp"

#include <iostream>
#include <sstream>

using namespace std;

TaskResultsFactory::TaskResultsFactory() {
}


TaskResultsFactory::~TaskResultsFactory() {
}


// create event source
TaskResults* TaskResultsFactory::create( const Settings* info )
{
    // loop over data source builders
    static map<string,Builder*>* bm = builderMap();
    map<string,Builder*>::iterator iter = bm->begin();
    map<string,Builder*>::iterator iend = bm->end();
    
    string mgrStr = info->get<string>("TaskResults");
    {
        while ( iter != iend )
        {
            const pair<string,Builder*>& element = *iter++;
            if (element.first == mgrStr)
            {
                return element.second->build( info );
            }
        }
    }
    return 0;
}


void TaskResultsFactory::registerBuilder( const string& name, Builder* b ) {
    static map<string,Builder*>& bm = *builderMap();
    bm[name] = b;
    return;
}

std::map<std::string,TaskResultsFactory::Builder*>* TaskResultsFactory::builderMap()
{
    static map<string,Builder*>* bm = new map<string,Builder*>;
    return bm;
}
