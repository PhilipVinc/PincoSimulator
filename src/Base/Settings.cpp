//
//  Settings.cpp
//  TWMC++
//
//  Created by Filippo Vicentini on 17/05/2017.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#include "Settings.hpp"

#include "CustomTypes.h"
#include "Utils/FsUtils.hpp"
#include "Libraries/FilesystemLibrary.h"
#include "NoisyMatrix.hpp"

#include <complex>
#include <iostream>
#include <map>
#include <string>
#include <time.h>


using namespace std;

#include <CLI/CLI.hpp>

Settings::Settings(int argc, char* argv[])
{
    cout << "--- Reading CLI Parameters..." << endl;

    CLI::App app("Pinco Simulator");

    filesystem::path inputPath; string input;
    filesystem::path outputPath; string output;
    bool append = false;
    CLI::Option *opti = app.add_option("-i,--input,input", input, "Input path")->required()->check(CLI::ExistingFile);
    CLI::Option *opto = app.add_option("-o,--output", output, "Output path");
    CLI::Option *opta = app.add_option("-a,--append", output, "Append results");
    app.allow_extras();

    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        //return app.exit(e);
        app.exit(e);
        exit(0);
        return;
    }

    inputPath = input;
    if (opto->count() > 0) {
        outputPath = output;
        outputFolderIsSet = true;
    }

    cout << "Will be looking at path: " << inputPath << endl;

    // Now that I have the path, I will check that it is okappa.
    
    // Check if it is a folder
    if (filesystem::is_directory(inputPath))
    {
        cout << "Input was a folder: " << inputPath << endl;
        outputPath = inputPath;
        inputPath = inputPath / paramsFileName;
        
        // Now check if the ini File exists.
        if (filesystem::exists(inputPath))
        {
            status = Status::subsequentRun;
        }
    }
    else if (filesystem::exists(inputPath))
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

        config = cpptoml::parse_file(inputPath.string());

        if (app.remaining_size() % 2 != 0) {
            cerr << "overridden parameters must have argument!" << endl;
            exit(0);
        } else if (app.remaining_size() != 0) {
            auto override_data = app.remaining();
            std::stringstream overrideEntries;
            cout << "-- Found " << app.remaining_size()/2 << " overridden parameters." << endl;
            for (int i = 0; i < app.remaining_size(); i+=2) {
                cout << "\t\t"  << override_data[i] << " = " << override_data[i+1] <<endl;
                overrideEntries << override_data[i] << " = " << override_data[i+1] <<endl;
            }
            cpptoml::parser p(overrideEntries);
            std::shared_ptr<cpptoml::table> g = p.parse();
            cout << *g << endl;
            config->merge(*g);
        }


        cout << "--- Printing .ini file content:" << endl;
        cout << *config << endl;
    }
    else
    {
        //std::cerr << desc << std::endl;
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
            rootFolder = get<std::string>("rootFolder", inputParentPathStr);
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
        result = config->get_as<T>(path);
    }
    catch(std::exception& exc)
    {
    }

    return result;
}

template <class T> inline T Settings::get(string path, T defRes) const
{
    T result;
    try
    {
        result = config->get_as<T>(path).value_or(defRes);
    }
    catch(std::exception& exc)
    {
    }

    return result;
}

template<>  string Settings::get<string>(string path) const
{
    string result = config->get_as<string>(path).value_or("");
    return result;
}

template<>  size_t Settings::get<size_t>(string path) const
{
    size_t result = config->get_as<size_t>(path).value_or(0);
    return result;
}

template<>  size_t Settings::get<size_t>(string path, size_t defVal ) const
{
    size_t result = config->get_as<size_t>(path).value_or(defVal);
    return result;
}

template<>  int Settings::get<int>(string path) const
{
    int result = config->get_as<int>(path).value_or(0);
    return result;
}

template<>  int Settings::get<int>(string path, int defVal) const
{
    int result = config->get_as<int>(path).value_or(defVal);
    return result;
}

template<>  float_p Settings::get<float_p>(string path) const
{
    float_p result = config->get_as<float_p>(path).value_or(0);
    return result;
}

template<>  bool Settings::get<bool>(string path) const
{
    bool result = config->get_as<bool>(path).value_or(false);
    return result;
}

template<>  bool Settings::get<bool>(string path, bool defVal) const
{
    bool result = config->get_as<bool>(path).value_or(defVal);
    return result;
}


template<>  std::complex<float> Settings::get<std::complex<float>>(string path, std::complex<float> def) const
{
    std::complex<float> result;
    try
    {
        //result = config->get_as<std::complex<float>>(path);
        result = std::complex<float>(float(config->get_as<double>(path).value_or(double(std::real(def)))),0);

    }
    catch(std::exception& exc)
    {
        float real = float(config->get_as<double>(path+"_real").value_or(std::real(def)));
        float imag = float(config->get_as<double>(path+"_imag").value_or(std::imag(def)));
        result = std::complex<float>(real, imag);
    }
    
    return result;
}
template<>  std::complex<float> Settings::get<std::complex<float>>(string path) const
{
    return get<std::complex<float>>(path, std::complex<float>(0,0));
}


template<>  std::complex<double> Settings::get<std::complex<double>>(string path, std::complex<double> def) const
{
    std::complex<double> result;
    try
    {
        //result = config->get_as<std::complex<double>>(path);
        result = std::complex<double>(config->get_as<double>(path).value_or(std::real(def)),0);
    }
    catch(std::exception& exc)
    {
        double real = config->get_as<double>(path+"_real").value_or(std::real(def));
        double imag = config->get_as<double>(path+"_imag").value_or(std::imag(def));
        result = std::complex<double>(real, imag);
    }
    
    return result;
}
template<>  std::complex<double> Settings::get<std::complex<double>>(string path) const
{
    return get<std::complex<double>>(path, std::complex<double>(0,0));
}



unsigned int Settings::GlobalSeed() const
{
    return config->get_as<unsigned int>("SEED").value_or(0) + (unsigned int) time(0);
}

vector<float_p> ReadCharFile(const string &path);
map<float_p,vector<float_p>> ReadTemporalCharFile(const string &path);

NoisyMatrix* Settings::GetMatrix(string path, size_t nx, size_t ny, size_t cellSz) const
{
    NoisyMatrix* result;
    result = new NoisyMatrix(nx, ny, cellSz);

	filesystem::path root = GetRootFolder();
    string matPath = config->get_as<string>(path).value_or("xxx");
    if (matPath != "xxx") {
        filesystem::path pathToFile = root / matPath;

        // check if it's  a file path, and import it if it is so:
        if (filesystem::exists(pathToFile))
        {
            vector<float_p> values = ReadCharFile(pathToFile.string());
            result->SetValue(values);
        }
    } else if (((get<complex_p>(path, complex_p(NAN, NAN)) != complex_p(NAN, NAN)))) {
        complex_p val = get<complex_p>(path, complex_p(NAN, NAN));
        if (val == complex_p(NAN, NAN))
        {
            cerr << "ERROR: The variable " << path << "is not set correctly." <<endl;
            cerr << "Probably you set a file, but the file does not exist." << endl;
        }
        result->SetValue(val);
    } else if((matPath = config->get_as<string>(path+"_t").value_or("xxx")) != "xxx")     // time dependence
    {
        if (filesystem::exists(root/matPath))
        {
            auto data = ReadTemporalCharFile((root/matPath).string());
            result->SetTemporalValue(data);
        }
    }
    else
    {
        cerr << "ERROR READING MATRIX @name: "<< path ;
        cerr << " @path:" << matPath << endl;
    }
    
    string noiseType = config->get_as<string>(path+"_Noise_Type").value_or("xxx");
    if (noiseType != "xxx")
    {
        result->SetNoiseType(noiseType);
    }
    size_t numOfNoiseVars = result->GetNoiseVariables();
    for (size_t i = 0; i != numOfNoiseVars; i++)
    {
        string noiseValPath = config->get_as<string>(path+"_Noise_Val_"+to_string(i)).value_or("xxx");
        complex_p val;
        if (noiseValPath != "xxx" && filesystem::exists(root / noiseValPath)) {
            filesystem::path pathToFile = root / noiseValPath;

            vector<float_p> values = ReadCharFile(pathToFile.string());
            result->SetNoiseVal(i, values);

        } else if (( val = get<complex_p>(path+"_Noise_Val_"+to_string(i)), complex_p(NAN,NAN)) != complex_p(NAN,NAN)){
            result->SetNoiseVal(i, val);
        } else {
            cerr << "ERROR: Variable " << path+"_Noise_Val_"+to_string(i) << " was not set, but was necessary." <<endl;
            cerr << "Aborting" << endl;
        }

    }
    return result;
}

map<float_p,vector<float_p>> ReadTemporalCharFile(const string &path)
{
    map<float_p, vector<float_p>> result;

    ifstream infile(path);
    string line; string st;

    float_p time; float_p tmp;

    while(getline(infile, line))
    {
        istringstream iss(line);
        iss >> time;
        vector<float_p> tmpArr;

        while (getline(infile, line))
        {
            if (line == ""|| line == "-" || line == " " || line == "\t" || line == "\n")
                break;
            istringstream iss(line);
            while (iss >> st)
            {
                if (st == "*" or st == "nan" or st == "NAN" or st == "NaN")
                {
                    tmpArr.push_back(NAN);
                }
                else
                    tmpArr.push_back(stof(st));
            }
        }
        result.emplace(time, tmpArr);
    }

    return result;
}

vector<float_p> ReadCharFile(const string &path)
{
    ifstream file(path, ios::in);
    //file.open(path, ios::in);
    vector<float_p> vec;
    float_p tmp;
    if (file.is_open())
    {
        while(file >> tmp)
            vec.push_back(tmp);
        file.close();
    }
    cout << "File had length: " << vec.size() << endl;
    return vec;
}



