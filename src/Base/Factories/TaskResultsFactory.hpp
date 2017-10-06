//
//  TaskResultsFactory.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 22/09/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TaskResultsFactory_hpp
#define TaskResultsFactory_hpp

#include <stdio.h>
#include <map>

class Settings;
class TaskResults;

class TaskResultsFactory {
    
public:
    
    TaskResultsFactory();
    virtual ~TaskResultsFactory();
    
    // create manager
    static TaskResults* create( const Settings* settings );
    
    // manager object builder
    class Builder {
    public:
        Builder( const std::string& name ) { registerBuilder( name, this ); }
        virtual ~Builder() {}
        virtual TaskResults* build( const Settings* settings ) = 0;
    };
    
private:
    
    TaskResultsFactory           ( const TaskResultsFactory& x );
    TaskResultsFactory& operator=( const TaskResultsFactory& x );
    
    static void registerBuilder( const std::string& name, Builder* b );
    static std::map<std::string,Builder*>* builderMap();
    
};

#endif /* TaskResultsFactory_hpp */
