//
//  Settings.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "Settings.hpp"
#include "CustomTypes.h"
#include "FsUtils.hpp"


#include <iostream>
#include <string>
#include <complex>
#include <time.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "boost/program_options.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#pragma clang pop


namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace fs = boost::filesystem;
using namespace std;

Settings::Settings(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
       desc.add_options()
        ("help", "Print help messages")
        ("input", po::value<string>(), "Required folder with _sim.ini or ini file.")
        ("output", po::value<string>(), "Output folder [optional]");
        po::variables_map vm;
    fs::path inputPath;
    fs::path outputPath;
    bool append = false;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc)
                  .style(boost::program_options::command_line_style::unix_style |
                         boost::program_options::command_line_style::allow_long_disguise)
                  .run(),vm);
        
        
        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            exit(0);
        }
        
        if (vm.count("input"))
        {
            inputPath = vm["input"].as<std::string>();
        }
        
        if (vm.count("output"))
        {
            outputPath = vm["output"].as<std::string>();
            outputFolderIsSet = true;
        }
        
        if (vm.count("append"))
        {
            append = true;
        }
    }
    catch (std::exception& exc)
    {
        std::cerr << "ERROR: " << exc.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        exit(0);
        
    }
    cout << "Will be looking at path: " << inputPath << endl;

    // Now that I have the path, I will check that it is okappa.
    
    // Check if it is a folder
    if (fs::is_directory(inputPath))
    {
        cout << "Input was a folder: " << inputPath << endl;
        outputPath = inputPath;
        inputPath = inputPath / paramsFileName;
        
        // Now check if the ini File exists.
        if (fs::exists(inputPath))
        {
            status = Status::subsequentRun;
        }
    }
    else if (fs::exists(inputPath))
    {
        inputParentPathStr = inputPath.parent_path().string();
        status = Status::firstRun;
    }
    
    // Load Everything
    if (status != Status::invalid)
    {
        basePath = inputPath.parent_path().string();
        if (basePath == "")
            basePath = ".";
        
        cout << "Setting output path to " << outputPath << endl;
        tree = new pt::ptree;
        pt::ini_parser::read_ini(inputPath.string(), *tree);
        
        std::vector<std::pair<std::string, std::string>> keyval_pairs;
        for (pt::ptree::value_type &keyval : tree->get_child("") )
        {
            std::string name = keyval.first;
            std::string color = keyval.second.data();
            keyval_pairs.push_back(std::make_pair(name, color));
            cout << name << "  =  " << color << endl;
        }
    }
    else
    {
        std::cerr << desc << std::endl;
        exit(0);
    }

    // Now I should parse the ini file at the input path
    
    ////////////////////
    /// READING      ///
    ////////////////////
    inputPathStr = inputPath.string();
    outputPathStr = outputPath.string();
}

Settings::~Settings()
{
    
}

string Settings::GetOutputFolder() const
{
    return outputPathStr;
}

string Settings::GetRootFolder() const
{
    string rootFolder;
    switch (status) {
        case subsequentRun:
            rootFolder = basePath;
            break;
            
        default:
            rootFolder = tree->get<string>("rootFolder", inputParentPathStr);
            if (rootFolder == "")
                rootFolder = ".";
            
            break;
    }
    return rootFolder;
}

bool Settings::IsOutputFolderSet() const
{
    return outputFolderIsSet;
}


template <class T> inline T Settings::get(string path) const
{
    T result;
    try
    {
        result = tree->get<T>(path);
    }
    catch(std::exception& exc)
    {
    }
    
    return result;
}

inline template<>  string Settings::get<string>(string path) const
{
    string result = tree->get<string>(path, "");
    return result;
}

inline template<>  size_t Settings::get<size_t>(string path) const
{
    size_t result = tree->get<size_t>(path, 0);
    return result;
}

inline template<>  int Settings::get<int>(string path) const
{
    int result = tree->get<int>(path, 0);
    return result;
}

inline template<>  float_p Settings::get<float_p>(string path) const
{
    float_p result = tree->get<float_p>(path, 0);
    return result;
}

inline template<>  bool Settings::get<bool>(string path) const
{
    bool result = tree->get<bool>(path, false);
    return result;
}

inline template<>  complex<float> Settings::get<complex<float>>(string path) const
{
    complex<float> result;
    try
    {
        result = tree->get<complex<float>>(path);
    }
    catch(std::exception& exc)
    {
        float real = tree->get<float>(path+"_real",0.0);
        float imag = tree->get<float>(path+"_imag",0.0);
        result = complex<float>(real, imag);
    }
    
    return result;
}

inline template<>  complex<double> Settings::get<complex<double>>(string path) const
{
    complex<double> result;
    try
    {
        result = tree->get<complex<double>>(path);
    }
    catch(std::exception& exc)
    {
        double real = tree->get<double>(path+"_real",0.0);
        double imag = tree->get<double>(path+"_imag",0.0);
        result = complex<double>(real, imag);
    }
    
    return result;
}

unsigned int Settings::GlobalSeed() const
{
    return tree->get<unsigned int>("SEED", 0) + (unsigned int) time(0);
}



//template<> MatrixCXd Settings::get<MatrixCXd>(string path)
//{
//
//}




