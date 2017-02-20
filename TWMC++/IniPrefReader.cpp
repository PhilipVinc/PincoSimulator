//
//  IniPrefReader.cpp
//  WFMC++
//
//  Created by Filippo Vicentini on 16/01/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "IniPrefReader.hpp"
#include <iostream>

using namespace std;

IniPrefReader::IniPrefReader()
{
    
}

IniPrefReader::IniPrefReader(std::string iniFilePath)
{
    OpenPrefFile(iniFilePath);
}

IniPrefReader::~IniPrefReader()
{
    ClosePrefFile();
}

void IniPrefReader::OpenPrefFile(std::string fileName)
{
    if (iniFileDataInMemory)
        ClosePrefFile();
    
    std::string optionValue;
    ifstream infile;
    infile.open(fileName.c_str());
    
    //Does the file exist?
    if (infile.is_open() != true)
    {
        return;
    }
    
    std::string key;
    
    while (!infile.eof()) // To get you all the lines.
    {
        getline(infile, optionValue); // Saves the line in STRING.
        
        //Is the option a comment
        if (optionValue.substr(0, 1) == "#")
        {
            continue;
        }
        
        key = parseOptionName(optionValue);
        
        if (key.length() > 0)
        {
            iniItem[i] = new ConfigItems;
            iniItem[i]->key = key;
            iniItem[i]->value = parseOptionValue(optionValue);
            i++;
        }
    }
    
    i--;
    infile.close();
    iniFileDataInMemory = true;
}

void IniPrefReader::ClosePrefFile()
{
    if (iniFileDataInMemory)
    {
        for (int x = 0; x <= i; x++)
        {
            delete iniItem[x];
        }
        i = 0;
    }
    iniFileDataInMemory = false;
}

std::string IniPrefReader::getOptionToString(std::string key)
{
    //Check to see if anything got parsed?
    if (i == 0)
    {
        return "";
    }
    
    for (int x = 0; x <= i; x++)
    {
        if (key == iniItem[x]->key)
        {
            return iniItem[x]->value;
        }
    }
    
    return "";
}

const char* IniPrefReader::getOptionToChar(std::string key)
{
    //Check to see if anything got parsed?
    if (i == 0)
    {
        return "";
    }
    
    for (int x = 0; x <= i; x++)
    {
        if (key == iniItem[x]->key)
        {
            return iniItem[x]->value.c_str();
        }
    }
    
    return "";
}

int IniPrefReader::getOptionToInt(std::string key)
{
    //Check to see if anything got parsed?
    if (i == 0)
    {
        return 0;
    }
    
    for (int x = 0; x <= i; x++)
    {
        if (key == iniItem[x]->key)
        {
            return atoi(iniItem[x]->value.c_str());
        }
    }
    
    return 0;
}

long double IniPrefReader::getOptionToFloat(std::string key)
{
    //Check to see if anything got parsed?
    if (i == 0)
    {
        return 0;
    }
    
    for (int x = 0; x <= i; x++)
    {
        if (key == iniItem[x]->key)
        {
            return std::stod(iniItem[x]->value.c_str());
        }
    }
    
    return 0;
}

/*std::vector<complexTriplet> IniPrefReader::getFileAsTripletList(std::string key)
{
    string fname = this->getOptionToString(key);
    std::vector<complexTriplet> tripletList;
    
    
    ifstream file(fname);
    int i, j;
    float_p real_val, imag_val;
    
    while (!file.eof())
    {
        file >>i;
        file >> j;
        file >> real_val;
        file >> imag_val;
        
        tripletList.push_back(complexTriplet(i,j,complex_p(real_val,imag_val)));
    }
    file.close();
    
    return tripletList;
}*/

float_p* IniPrefReader::getValueOrMatrixReal(std::string key, size_t nx, size_t ny)
{
    string fname = this->getOptionToString(key);
    
    size_t size = nx*ny;
    float_p* resMatrix = new float_p[size];
    if (fname == "")
    {
        for(size_t i=0; i!= size; i++)
        {
            resMatrix[i] = 0.0;
        }
    }
    else
    {
        ifstream file(fname);
        float_p tmp;
        size_t j = 0;
        while(!file.eof() && j < size)
        {
            file >> tmp;
            resMatrix[j] = tmp;
            j++;
        }
        file.close();
    }
    
    cout << key << " = ";
    for(size_t i=0; i!= size; i++)
    {
        cout << resMatrix[i] << "\t";
    }
    cout << endl;
    return resMatrix;
}


complex_p* IniPrefReader::getValueOrMatrixImag(std::string key, size_t nx, size_t ny)
{
    string fname = this->getOptionToString(key);
    
    size_t size = nx*ny;
    complex_p* resMatrix = new complex_p[size];
    if (fname == "")
    {
        for(size_t i=0; i!= size; i++)
        {
            resMatrix[i] = 0;
        }
    }
    else
    {
        ifstream file(fname);
        float_p tmpR, tmpI;
        size_t j = 0;
        while(!file.eof())
        {
            file >> tmpR >> tmpI;
            resMatrix[j] = complex_p(tmpR,tmpI);
            j++;
        }
        file.close();
    }
    return resMatrix;
}



std::string IniPrefReader::parseOptionName(std::string value)
{
    size_t found;
    
    found = value.find('=');
    
    if (found > 100)
    {
        return "";
    }
    
    std::string key = value.substr(0, (found-1));
    key = trim(key);
    
    return key;
}

std::string IniPrefReader::parseOptionValue(std::string value)
{
    size_t found;
    
    found = value.find('=');
    
    if (found > 100)
    {
        return "";
    }
    
    std::string keyValue = value.substr((found+1));
    keyValue = trim(keyValue);
    
    return keyValue;
}

std::string IniPrefReader::trim(std::string s)
{
    return ltrim(rtrim(s));
}

// trim from start
std::string IniPrefReader::ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
std::string IniPrefReader::rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}


