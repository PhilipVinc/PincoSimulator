//
//  IniPrefReader.hpp
//  WFMC++
//
//  Created by Filippo Vicentini on 16/01/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef IniPrefReader_hpp
#define IniPrefReader_hpp

#include <stdio.h>
#include <string>
#include <fstream>
#include <algorithm>
#include "TWMC_structures.h"

class IniPrefReader
{
public:
    
    IniPrefReader();
    IniPrefReader(std::string iniFilePath);
    ~IniPrefReader();
    
    void OpenPrefFile(std::string iniFilePath);
    void ClosePrefFile();
    
    /**
     * Parse a configuration file
     *
     * @param	fileName The name of the file to parse
     */
    void parseIniFile(std::string fileName);
    
    /**
     * If you are finished with the config item, use this function to cleanup the results
     *
     */
    void cleanupIniReader();
    
    /**
     * Return the value of the requested key in with the string type
     *
     * @param	key The option key
     * @return	string The value of the requested key
     */
    std::string getOptionToString(std::string key);
    
    /**
     * Return the value of the requested key in with the int type
     *
     * @param	key The option key
     * @return	int The value of the requested key
     * @note	If item is not an integer (or does not exist) then 0 will be returned.
     */
    int getOptionToInt(std::string key);
    
    /**
     * Return the value of the requested key in with the long double type
     *
     * @param	key The option key
     * @return	int The value of the requested key
     * @note	If item is not an integer (or does not exist) then 0 will be returned.
     */
    
    long double getOptionToFloat(std::string key);
    
    /**
     * Return the value of the requested key in with the char type
     *
     * @param    key The option key
     * @return   char The value of the requested key
     */
    const char *getOptionToChar(std::string key);
    
    //std::vector<complexTriplet> getFileAsTripletList(std::string key);
    
    float_p* getValueOrMatrixReal(std::string key, size_t nx, size_t ny);
    complex_p* getValueOrMatrixImag(std::string key, size_t nx, size_t ny);
    
    
    std::string parseOptionName(std::string value);
    std::string parseOptionValue(std::string value);
    
private:
    struct ConfigItems {
        std::string key;
        std::string value;
    };
    ConfigItems* iniItem[1024];
    int i = 0;
    
    std::string trim(std::string s);
    std::string rtrim(std::string s);
    std::string ltrim(std::string s);
    
    bool iniFileDataInMemory = false;
};



#endif /* IniPrefReader_hpp */
