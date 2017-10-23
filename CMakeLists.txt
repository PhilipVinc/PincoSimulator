cmake_minimum_required(VERSION 3.8)
project(Simulator)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
add_definitions(-std=c++14 -stdlib=libc++)

#set(CMAKE_VERBOSE_MAKEFILE ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

If(LINUX)
    message(Linux Detected)
    message(is: ${CMAKE_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -stdlib=libc++")
endif()

# Directory with all compilation modules
set(CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake-modules)

# Find package boost, if present
if(APPLE)
    find_package(Boost 1.65 COMPONENTS program_options system filesystem)
    if (NOT Boost_FOUND)
        message(Searching again for Boost)
        SET(BOOST_ROOT ~/opt/local/)
        SET(BOOST_ROOT ~/opt/local/)
        find_package(Boost 1.65 COMPONENTS program_options system filesystem REQUIRED )
    endif (NOT Boost_FOUND)
else()
    find_package(Boost 1.65 COMPONENTS program_options system )
    if (NOT Boost_FOUND)
        message(Searching again for Boost)
        SET(BOOST_ROOT ~/opt/local/)
        SET(BOOST_ROOT ~/opt/local/)
        find_package(Boost 1.65 COMPONENTS program_options system REQUIRED )
    endif (NOT Boost_FOUND)
endif()

message(found ${Boost_INCLUDE_DIR})
message(found ${Boost_LIBRARIES})


find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIR})

message(found ${FFTW_INCLUDE_DIR})
message(found ${FFTW_LIBRARIES})



set(LINKING SHARED)

# Src Folders
include_directories(src)

include_directories(src/Base/)
include_directories(src/Base/FileFormats/)
include_directories(src/Base/Utils/)
include_directories(src/Libraries/)
include_directories(src/ThreadedManager/)
include_directories(src/TWMC/)
include_directories(src/TWMC/SimpleTWMC/)
include_directories(src/TWMC/ThermoTWMC/)
include_directories(src/TWMC/AppendTWMC/)

set(BASE_SOURCES
        #        src/Base/Factories/ManagerFactory.cpp
        #        src/Base/Factories/ManagerFactory.hpp
        #        src/Base/Factories/TaskResultsFactory.cpp
        #        src/Base/Factories/TaskResultsFactory.hpp
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
        src/Libraries/FilesystemLibrary.h
        src/Libraries/TStaticFactory.h)

add_library(SimFramework ${LINKING} ${BASE_SOURCES})
target_link_libraries(SimFramework m)
target_link_libraries(SimFramework ${Boost_LIBRARIES})

if(LINUX)
    target_link_libraries(SimFramework rt) #realtime
    target_link_libraries(SimFramework c++experimental) #filesystem
endif()


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
target_link_libraries(TWMCFramework m)
target_link_libraries(TWMCFramework ${FFTW_LIBRARIES})
target_link_libraries(TWMCFramework ${Boost_LIBRARIES})

set(EXE_FILE
        src/main.cpp)

add_executable(simexe ${EXE_FILE})
target_link_libraries(simexe SimFramework)
target_link_libraries(simexe TWMCFramework)


set(SOURCE_FILES
        #src/Base/Factories/ManagerFactory.cpp
        #src/Base/Factories/ManagerFactory.hpp
        #src/Base/Factories/TaskResultsFactory.cpp
        #src/Base/Factories/TaskResultsFactory.hpp
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
        src/main.cpp src/Libraries/FilesystemLibrary.h
        src/Libraries/TStaticFactory.h)

add_executable(sim ${SOURCE_FILES})

# Includes
include_directories(/usr/local/include)
include_directories(/usr/local/include/eigen3/)


# Linker
target_link_libraries(sim ${Boost_LIBRARIES})
target_link_libraries(sim ${FFTW_LIBRARIES})
target_link_libraries(sim m)

#target_link_libraries(sim fftw_thread)

if(LINUX)
    target_link_libraries(sim rt) #realtime
    target_link_libraries(sim c++experimental) #filesystem
endif()


# Copy file
add_custom_command(TARGET sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:sim>/TestSimulation)

add_custom_command(TARGET sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/SampleData/ $<TARGET_FILE_DIR:sim>/TestSimulation/)
