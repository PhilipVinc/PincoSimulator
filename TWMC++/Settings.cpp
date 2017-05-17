//
//  Settings.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include <iostream>
#include <string>
#include <complex>


#include "boost/program_options.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

#include "Settings.hpp"
#include "FsUtils.hpp"
#include "CustomTypes.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

using namespace std;
const string paramsFileName = "_sim.ini";
const string trajectoryFolderBaseName = "trajectories";

Settings::Settings(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
       desc.add_options()
        ("help", "Print help messages")
        ("input", po::value<string>(), "additional options")
        ("output", po::value<string>(), "this");
        po::variables_map vm;
    fs::path inputPath;
    fs::path outputPath;

    try
    {

        po::store(po::command_line_parser(argc, argv).options(desc)
                  .style(boost::program_options::command_line_style::unix_style |
                         boost::program_options::command_line_style::allow_long_disguise)
                  .run(),vm);
        
        
        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return;
        }
        
        if (vm.count("input"))
        {
            inputPath = vm["input"].as<std::string>();
        }
        
        if (vm.count("output"))
        {
            outputPath = vm["output"].as<std::string>();
        }
    }
    catch (std::exception& exc)
    {
        std::cerr << "ERROR: " << exc.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return;
        
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
        status = Status::firstRun;
    }
    
    // Load Everything
    if (status != Status::invalid)
    {
        basePath = inputPath.parent_path().string();
        if (basePath == "")
            basePath = ".";
        auto trajectoryFoldersN = findFoldersContainingString(basePath, trajectoryFolderBaseName).size()+1;
        outputPath = inputPath.parent_path()  / (trajectoryFolderBaseName + to_string(trajectoryFoldersN));
        
        cout << "Setting output path to " << outputPath << endl;
        tree = new pt::ptree;
        pt::ini_parser::read_ini(inputPath.string(), *tree);
        
        std::vector< std::pair<std::string, std::string> > animals;
        for (pt::ptree::value_type &animal : tree->get_child("") )
        {
            std::string name = animal.first;
            std::string color = animal.second.data();
            animals.push_back(std::make_pair(name, color));
            cout << name << "  =  " << color << endl;
        }
    }

    // Now I should parse the ini file at the input path
    
    ////////////////////
    /// READING      ///
    ////////////////////
    inputPathStr = inputPath.string();
    outputPathStr = outputPath.string();
    SetupOutputFolder();
}

Settings::~Settings()
{
    
}

void Settings::SetupOutputFolder()
{
    CheckCreateFolder(outputPathStr);
}




template <typename T> T Settings::get(string path)
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

template<> string Settings::get<string>(string path)
{
    string result = tree->get<string>(path, "");
    return result;
}

template<> bool Settings::get<bool>(string path)
{
    bool result = tree->get<bool>(path, false);
    return result;
}

template<> complex<float> Settings::get<complex<float>>(string path)
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

template<> complex<double> Settings::get<complex<double>>(string path)
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



//template<> MatrixCXd Settings::get<MatrixCXd>(string path)
//{
//
//}




