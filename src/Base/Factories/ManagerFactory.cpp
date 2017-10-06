//
//  ManagerFactory.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 23/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//
#include <string>

#include "ManagerFactory.hpp"
#include "Manager.hpp"
#include "Settings.hpp"

#include <iostream>
#include <sstream>

using namespace std;

ManagerFactory::ManagerFactory() {
}


ManagerFactory::~ManagerFactory() {
}


// create event source
Manager* ManagerFactory::create( const Settings* info )
{
    // loop over data source builders
    static map<string,Builder*>* bm = builderMap();
    map<string,Builder*>::iterator iter = bm->begin();
    map<string,Builder*>::iterator iend = bm->end();
    /*while ( iter != iend ) {
        const pair<string,Builder*>& element = *iter++;
        // create analysis object if requested
        if ( info->contains( element.first ) )
            return element.second->build( info );
    }*/
    
    string mgrStr = info->get<string>("Manager");
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


void ManagerFactory::registerBuilder( const string& name, Builder* b ) {
    static map<string,Builder*>& bm = *builderMap();
    bm[name] = b;
    return;
}

std::map<std::string,ManagerFactory::Builder*>* ManagerFactory::builderMap()
{
    static map<string,Builder*>* bm = new map<string,Builder*>;
    return bm;
}

