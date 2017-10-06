cmake_minimum_required(VERSION 3.8)
project(TWMC__Heun_Ref)

set(CMAKE_CXX_STANDARD 11)

# Src Folders
include_directories(src)

include_directories(src/Base/)
include_directories(src/Base/Factories/)
include_directories(src/Base/FileFormats/)
include_directories(src/Base/Utils/)
include_directories(src/Libraries/)
include_directories(src/ThreadedManager/)
include_directories(src/TWMC/)
include_directories(src/TWMC/SimpleTWMC/)
include_directories(src/TWMC/ThermoTWMC/)
include_directories(src/TWMC/AppendTWMC/)

set(SOURCE_FILES
        src/Base/Factories/ManagerFactory.cpp
        src/Base/Factories/ManagerFactory.hpp
        src/Base/Factories/TaskResultsFactory.cpp
        src/Base/Factories/TaskResultsFactory.hpp
        src/Base/FileFormats/ChunkFileSet.cpp
        src/Base/FileFormats/ChunkFileSet.hpp
        src/Base/FileFormats/ChunkRegister.cpp
        src/Base/FileFormats/ChunkRegister.hpp
        src/Base/FileFormats/DataStore.cpp
        src/Base/FileFormats/DataStore.hpp
        src/Base/FileFormats/PincoFormatDataStore.cpp
        src/Base/FileFormats/PincoFormatDataStore.hpp
        src/Base/Utils/EigenUtils.cpp
        src/Base/Utils/EigenUtils.hpp
        src/Base/Utils/FsUtils.cpp
        src/Base/Utils/FsUtils.hpp
        src/Base/CustomTypes.h
        src/Base/DataSaver.cpp
        src/Base/DataSaver.hpp
        src/Base/Manager.cpp
        src/Base/Manager.hpp
        src/Base/NoisyMatrix.cpp
        src/Base/NoisyMatrix.hpp
        src/Base/Settings.cpp
        src/Base/Settings.hpp
        src/Base/Task.cpp
        src/Base/Task.hpp
        src/Base/TaskData.cpp
        src/Base/TaskData.hpp
        src/Base/TaskResults.cpp
        src/Base/TaskResults.hpp
        src/Libraries/concurrentqueue.h
        src/ThreadedManager/ThreadManager.cpp
        src/ThreadedManager/ThreadManager.hpp
        src/ThreadedManager/WorkerThread.cpp
        src/ThreadedManager/WorkerThread.hpp
        src/TWMC/AppendTWMC/TWMCAppendThreadManager.cpp
        src/TWMC/AppendTWMC/TWMCAppendThreadManager.hpp
        src/TWMC/SimpleTWMC/TWMCResults.cpp
        src/TWMC/SimpleTWMC/TWMCResults.hpp
        src/TWMC/SimpleTWMC/TWMCSimData.cpp
        src/TWMC/SimpleTWMC/TWMCSimData.hpp
        src/TWMC/SimpleTWMC/TWMCSimulation.cpp
        src/TWMC/SimpleTWMC/TWMCSimulation.hpp
        src/TWMC/ThermoTWMC/TWMCThermoResults.cpp
        src/TWMC/ThermoTWMC/TWMCThermoResults.hpp
        src/TWMC/ThermoTWMC/TWMCThermoSimulation.cpp
        src/TWMC/ThermoTWMC/TWMCThermoSimulation.hpp
        src/TWMC/ThermoTWMC/TWMCThermoThreadManager.cpp
        src/TWMC/ThermoTWMC/TWMCThermoThreadManager.hpp
        src/TWMC/TrajectorySaver.cpp
        src/TWMC/TrajectorySaver.hpp
        src/TWMC/TWMCThreadManager.cpp
        src/TWMC/TWMCThreadManager.hpp
        src/TWMC/TWMCTypes.h
        src/main.cpp)

add_executable(sim ${SOURCE_FILES})

# Includes
include_directories(/usr/local/include)
include_directories(/usr/local/include/eigen3/)


# Linker
target_link_libraries(sim fftw3)
target_link_libraries(sim fftw3_threads)
target_link_libraries(sim m)
target_link_libraries(sim boost_program_options)
target_link_libraries(sim boost_filesystem)
target_link_libraries(sim boost_system)
