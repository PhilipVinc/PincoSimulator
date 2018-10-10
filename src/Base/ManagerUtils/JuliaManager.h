//
// Created by Filippo Vicentini on 27/08/18.
//

#ifndef SIMULATOR_JULIAMANAGER_H
#define SIMULATOR_JULIAMANAGER_H

#include "Base/Manager.hpp"

#include <memory>
#include <random>
#include <string>
#include <thread>

class DataStore;
class ResultsSaver;
class ProgressReporter;

class JuliaManager : public Manager {
public:
    explicit JuliaManager(const Settings* settings);
    ~JuliaManager() final;

    void ManagerLoop() final;

protected:
    void Setup();

private:
    std::shared_ptr<DataStore> _dataStore;
    std::unique_ptr<ResultsSaver> _saver;

    std::string juliaDataFolder;
    size_t nx;
    size_t ny;
    size_t n_frames;
    double t_start;
    double t_end;
    bool execute = true;

    std::unique_ptr<ProgressReporter> _progressReporter;
    std::string solverName;
};

static ManagerFactory::Registrator<JuliaManager> JuliaManagerReg =
        ManagerFactory::Registrator<JuliaManager>("julia");



#endif //SIMULATOR_JULIAMANAGER_H
