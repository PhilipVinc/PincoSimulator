cmake_minimum_required(VERSION 3.8)
project(Simulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(LINKING SHARED)

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


set(BASE_SOURCES
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
        src/Libraries/concurrentqueue.h src/Libraries/FilesystemLibrary.h)

add_library(SimFramework ${LINKING} ${BASE_SOURCES})
target_link_libraries(SimFramework m)
target_link_libraries(SimFramework boost_program_options)
target_link_libraries(SimFramework boost_filesystem)
target_link_libraries(SimFramework boost_system)

set(TWMC_SOURCES
        src/ThreadedManager/ThreadManager.cpp
        src/ThreadedManager/ThreadManager.hpp
        src/ThreadedManager/WorkerThread.cpp
        src/ThreadedManager/WorkerThread.hpp
        src/TWMC/AppendTWMC/TWMCAppendThreadManager.cpp
        src/TWMC/AppendTWMC/TWMCAppendThreadManager.hpp
        src/TWMC/SimpleTWMC/TWMCResults.cpp
        src/TWMC/SimpleTWMC/TWMCResults.hpp
        src/TWMC/TWMCSimData.cpp
        src/TWMC/TWMCSimData.hpp
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
        src/TWMC/SimpleTWMC/TWMCThreadManager.cpp
        src/TWMC/SimpleTWMC/TWMCThreadManager.hpp
        src/TWMC/TWMCTypes.h)

add_library(TWMCFramework ${LINKING} ${TWMC_SOURCES})
target_link_libraries(TWMCFramework SimFramework)
target_link_libraries(TWMCFramework fftw3)
target_link_libraries(TWMCFramework fftw3_threads)
target_link_libraries(TWMCFramework m)
target_link_libraries(TWMCFramework boost_program_options)
target_link_libraries(TWMCFramework boost_filesystem)
target_link_libraries(TWMCFramework boost_system)

set(EXE_FILE
        src/main.cpp)

add_executable(simexe ${EXE_FILE})
target_link_libraries(simexe SimFramework)
target_link_libraries(simexe TWMCFramework)


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
        src/TWMC/TWMCSimData.cpp
        src/TWMC/TWMCSimData.hpp
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
        src/TWMC/SimpleTWMC/TWMCThreadManager.cpp
        src/TWMC/SimpleTWMC/TWMCThreadManager.hpp
        src/TWMC/TWMCTypes.h
        src/main.cpp src/Libraries/FilesystemLibrary.h)

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

# Copy file
add_custom_command(TARGET sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:sim>/TestSimulation)

add_custom_command(TARGET sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/SampleData/ $<TARGET_FILE_DIR:sim>/TestSimulation/)
#file(COPY Support Files/test-sim.ini DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})