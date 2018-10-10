//
// Created by Filippo Vicentini on 27/08/18.
//

#include "JuliaManager.h"

#include "Base/FileFormats/PincoFormat/PincoFormatDataStore.hpp"
#include "Base/TaskResults.hpp"

#include "Base/Modules/ProgressReporter.hpp"
#include "Base/Modules/ResultsSaver.hpp"
#include "Base/TaskResults.hpp"

#include "TWMC/TWMCResults.hpp"

#include "Libraries/FilesystemLibrary.h"
#include "easylogging++.h"

#include <chrono>
#include <limits>
#include <memory>
#include <vector>

JuliaManager::JuliaManager(const Settings *settings) : Manager(settings) {
  LOG(INFO) << "Creating Resort Manager. ";
  t_end = settings->get<float_p>("t_end");
}

JuliaManager::~JuliaManager() {}

void JuliaManager::Setup() {
  juliaDataFolder = settings->get<std::string>("folder");
  if (juliaDataFolder != "") {
    LOG(INFO) << "Loading data from: " << settings->get<std::string>("folder");
  } else {
    juliaDataFolder = settings->GetRootFolder();
    LOG(INFO) << "Loading data from (alternative): "
              << settings->get<std::string>("folder");
  }
  auto foldPath = filesystem::path(juliaDataFolder + "/jbindata");
  size_t nFiles = std::count_if(filesystem::directory_iterator(foldPath),
                                filesystem::directory_iterator(),
                                static_cast<bool (*)(const filesystem::path &)>(
                                    filesystem::is_regular_file));

  size_t nTrajs = nFiles / 2;
  LOG(INFO) << "There are " << nFiles << " files, so " << nTrajs
            << " trajectories.";


  auto n_traj_ini = settings->get<size_t>("n_traj");
  LOG(INFO) << "There should be  " << n_traj_ini << " trajectories";
  if (nTrajs < n_traj_ini)
  {
    std::cout << "There are "<< nTrajs<< " trajectories instead of " << n_traj_ini<< " that there should be.";
    std::cout << "Proceed anyway? [0/1]" << std::endl;
    
    std::cin >> execute;
  }
  if (!execute)
    return;


  LOG(INFO) << "Saving data to: " << settings->GetOutputFolder();
  std::shared_ptr<PincoFormatDataStore> ds_new =
      std::make_shared<PincoFormatDataStore>(settings,
                                             settings->GetOutputFolder());

  _dataStore = std::static_pointer_cast<DataStore>(ds_new);
  auto path1 = filesystem::path(juliaDataFolder)/filesystem::path("_t.dat");
  auto path2 = filesystem::path(juliaDataFolder)/filesystem::path("data/_t.dat");

  filesystem::copy_file(path1, path2);
  //filesystem::copy(path1,
   //                path2);
  LOG(INFO) << "succesfully created archives";
  _saver = std::make_unique<ResultsSaver>(settings, _dataStore);
}

void JuliaManager::ManagerLoop() {
  if (!execute)
    return;

  auto foldPath = filesystem::path(juliaDataFolder + "/jbindata");
  size_t nFiles = std::count_if(filesystem::directory_iterator(foldPath),
                                filesystem::directory_iterator(),
                                static_cast<bool (*)(const filesystem::path &)>(
                                    filesystem::is_regular_file));

  size_t nTrajs = nFiles / 2;
  LOG(INFO) << "There are " << nFiles << " files, so " << nTrajs
            << " trajectories.";

  // Compute length and stuff:
  {
    size_t i = 1;
    std::string noisepath =
        foldPath.string() + "/start_" + std::to_string(i) + ".bin32";
    FILE *omegaFile = fopen(noisepath.c_str(), "rb");
    auto start      = ftell(omegaFile);
    fseek(omegaFile, 0, SEEK_END);
    auto end = ftell(omegaFile);
    fclose(omegaFile);

    auto dim = (end - start) / 4;
    nx       = sqrt(dim);
    ny       = nx;

    std::string datapath =
        foldPath.string() + "/field_" + std::to_string(i) + ".bin32";
    FILE *dataFile = fopen(datapath.c_str(), "rb");
    start          = ftell(dataFile);
    fseek(dataFile, 0, SEEK_END);
    end = ftell(dataFile);

    n_frames = (end - start) / 4 / 2 / dim;
    t_start  = 0;

    LOG(INFO) << "Insert nx:" << nx;
    LOG(INFO) << "Insert ny:" << ny;
    LOG(INFO) << "Insert n_frames: " << n_frames;
    LOG(INFO) << "Insert t_start: " << t_start;
    LOG(INFO) << "Insert t_end: " << t_end;
  }

  int nQueuedTrajs = 0;
  for (size_t i = 1; nQueuedTrajs < nTrajs; i++) {

    std::string tmpPath =
        foldPath.string() + "/start_" + std::to_string(i) + ".bin32";
    filesystem::path fpath = filesystem::path(tmpPath);
    if (!filesystem::is_regular_file(fpath)) { continue; }

    TWMCResults *res = new TWMCResults();
    res->SetId(i - 1);

    // Copy the data from omega (noise)
    float *omegaData = new float[nx * ny];

    std::string noisepath =
        foldPath.string() + "/start_" + std::to_string(i) + ".bin32";
    FILE *omegaFile = fopen(noisepath.c_str(), "rb");
    fread(omegaData, sizeof(float), nx * ny, omegaFile);
    fclose(omegaFile);

    MatrixCXd omega = MatrixCXd::Zero(nx, ny);
    auto omega_ptr  = omega.data();
    for (size_t j = 0; j < nx * ny; j++) { omega_ptr[j] = omegaData[j]; }
    delete[] omegaData;

    res->AddDataset<MatrixCXd>(TWMCData::Delta_Noise, omega, 1, {nx, ny});

    // Copy the data of the complex field
    std::complex<float> *fieldData =
        new std::complex<float>[nx * ny * n_frames];

    std::string datapath =
        foldPath.string() + "/field_" + std::to_string(i) + ".bin32";
    FILE *dataFile = fopen(datapath.c_str(), "rb");
    fread(fieldData, sizeof(std::complex<float>), nx * ny * n_frames, dataFile);
    fclose(dataFile);

    std::vector<complex_p> res_betat(nx * ny * n_frames);
    for (size_t j = 0; j < nx * ny * n_frames; j++) {
      res_betat[j] = fieldData[j];
    }
    delete[] fieldData;

    res->AddDataset<std::vector<complex_p>>(
        TWMCData::traj, std::move(res_betat), n_frames, {nx, ny});

    res->extraDataMemory[0] = t_start;
    res->extraDataMemory[1] = t_end;

    std::vector<std::unique_ptr<TaskResults>> allResults;
    allResults.push_back(std::unique_ptr<TaskResults>(res));
    // End of generating allResults

    size_t savedItems = _saver->SavedItems();
    std::cout << " Saved : " << savedItems << "/" << nTrajs
              << " - in queue : " << nQueuedTrajs - savedItems << "           "<<std::endl;
    std::cout << "\x1b[A\r" << std::flush;

    _saver->EnqueueTasks(std::move(allResults));
    nQueuedTrajs++;
    size_t nInMemory = nQueuedTrajs - savedItems;

    if (nInMemory > 30) {
      while(nInMemory > 10) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        size_t savedItems = _saver->SavedItems();
        std::cout << " Saved : " << savedItems << "/" << nTrajs
                  << " - in queue : " << nQueuedTrajs - savedItems << " (stalling)"<<std::endl;
        std::cout << "\x1b[A\r" << std::flush;
        nInMemory = nQueuedTrajs - savedItems;

      }
    }

  }

  while (_saver->SavedItems() < nTrajs) {
    _saver->Update();
    LOG(INFO) << "Saved " << _saver->SavedItems() << " items.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  LOG(INFO) << "Saved " << _saver->SavedItems() << " items.";
}
