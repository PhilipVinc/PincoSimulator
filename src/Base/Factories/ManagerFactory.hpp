//
//  ManagerFactory.hpp
//  TWMC++
//
//  Created by Filippo Vicentini on 23/05/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef ManagerFactory_hpp
#define ManagerFactory_hpp

#include <string>
#include <map>


class Settings;
class Manager;

class ManagerFactory {
    
public:
    
    ManagerFactory();
    virtual ~ManagerFactory();
    
    // create manager
    static Manager* create( const Settings* settings );
    
    // manager object builder
    class Builder {
    public:
        Builder( const std::string& name ) { registerBuilder( name, this ); }
        virtual ~Builder() {}
        virtual Manager* build( const Settings* settings ) = 0;
    };
    
private:
    
    ManagerFactory           ( const ManagerFactory& x );
    ManagerFactory& operator=( const ManagerFactory& x );
    
    static void registerBuilder( const std::string& name, Builder* b );
    static std::map<std::string,Builder*>* builderMap();
    
};

#endif /* ManagerFactory_hpp */
