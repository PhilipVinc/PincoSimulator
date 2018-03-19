cmake_minimum_required(VERSION 3.8)
project(Simulator)

#set(COMPILE_MPI ON)

option(COMPILE_GPU "Build with GPU Support for the Solvers" OFF)
option(COMPILE_MPI "Build with MPI Support for the Manager" OFF)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native")
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
set(BOOST_PACKAGES program_options system filesystem)

# GPU SUPPORT
if (COMPILE_GPU)
    message("Enabling GPU Support")
    add_definitions(-DGPU_SUPPORT)

    #VEXCL Definitions
    option(VEXCL_SHOW_KERNELS "Show generated kernels in tests and examples" OFF)
    option(VEXCL_CACHE_KERNELS "Cache compiled kernels offline" ON)

    if (CMAKE_VERSION VERSION_LESS "3.1.0")
        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/opencl)
    endif()
    list(APPEND BOOST_PACKAGES chrono date_time thread )
    message("${BOOST_PACKAGES}")

    # VexCL Backends
    add_library(Common INTERFACE)
    add_library(VexCL::Common ALIAS Common)

    find_package(OpenCL)
    if(OpenCL_FOUND)
        add_library(OpenCL INTERFACE)
        add_library(VexCL::OpenCL ALIAS OpenCL)

        target_include_directories(OpenCL INTERFACE ${OpenCL_INCLUDE_DIRS})
        target_link_libraries(OpenCL INTERFACE Common ${OpenCL_LIBRARIES})
        target_compile_definitions(OpenCL INTERFACE VEXCL_BACKEND_OPENCL)

        message(STATUS "Found VexCL::OpenCL")

    endif()
endif(COMPILE_GPU)

if (COMPILE_MPI)
    message("Enabling MPI Support")
    add_definitions(-DMPI_SUPPORT)

    # New interface
    # add_library(cereal INTERFACE)
    find_package(cereal REQUIRED)

    # Find the mpi implementation linker flags and libraries (this is not the best way
    # because I should be changing the compiler...
    execute_process(COMMAND mpic++ --showme:compile OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE mpi_compile_flags)
    execute_process(COMMAND mpic++ --showme:link OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE mpi_linker_flags)
    message("MPIC++ Compiler flags: ${mpi_compile_flags}")
    message("MPIC++ Linker flags: ${mpi_linker_flags}")

    # Append compile flags
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ${mpi_compile_flags} ")

    # Append Linker flags (I could also set flags per-target...
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${mpi_linker_flags}")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lompitrace")
    #message("MPIC++ Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")

    #set_target_properties(sim PROPERTIES LINK_FLAGS "${mpi_linker_flags}")

endif (COMPILE_MPI)

# Find package boost, if present
if(APPLE)
    find_package(Boost 1.65 COMPONENTS "${BOOST_PACKAGES}")
    if (NOT Boost_FOUND)
        message(Searching again for Boost)
        SET(BOOST_ROOT ~/opt/local/)
        SET(BOOST_ROOT ~/opt/local/)
        find_package(Boost 1.65 COMPONENTS "${BOOST_PACKAGES}" REQUIRED )
    endif (NOT Boost_FOUND)
else()
    find_package(Boost 1.65 COMPONENTS "${BOOST_PACKAGES}" )
    if (NOT Boost_FOUND)
        message("Searching again for Boost without filesystem support")
        SET(BOOST_ROOT ~/opt/local/)
        SET(BOOST_ROOT ~/opt/local/)
        list(REMOVE_ITEM BOOST_PACKAGES filesystem) # remove filesystem
        find_package(Boost 1.65 COMPONENTS "${BOOST_PACKAGES}" REQUIRED )
    endif (NOT Boost_FOUND)
endif()
include_directories(${Boost_INCLUDE_DIR})

message("found Boost_INCLUDE_DIR=${Boost_INCLUDE_DIR}")
message("found Boost_LIBRARIES=${Boost_LIBRARIES}")


find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIR})

message("found FFTW_INCLUDE_DIR=${FFTW_INCLUDE_DIR}")
message("found FFTW_LIBRARIES=${FFTW_LIBRARIES}")



set(LINKING STATIC)

# Src Folders
include_directories(src)

#include_directories(src/Base/)
#include_directories(src/TWMC/)


set(SOURCE_FILES
        src/Base/FileFormats/ChunkFileSet.cpp src/Base/FileFormats/ChunkFileSet.hpp
        src/Base/FileFormats/ChunkRegister.cpp src/Base/FileFormats/ChunkRegister.hpp
        src/Base/FileFormats/DataStore.cpp src/Base/FileFormats/DataStore.hpp
        src/Base/FileFormats/PincoFormatDataStore.cpp src/Base/FileFormats/PincoFormatDataStore.hpp
        src/Base/Utils/EigenUtils.cpp src/Base/Utils/EigenUtils.hpp
        src/Base/Utils/FsUtils.cpp src/Base/Utils/FsUtils.hpp
        src/Base/CustomTypes.h
        src/Base/Manager.cpp src/Base/Manager.hpp
        src/Base/NoisyMatrix.cpp src/Base/NoisyMatrix.hpp
        src/Base/Settings.cpp src/Base/Settings.hpp
        src/Base/TaskData.cpp src/Base/TaskData.hpp
        src/Base/TaskResults.cpp src/Base/TaskResults.hpp
        src/Libraries/concurrentqueue.h
        src/Base/ThreadedTaskProcessor/WorkerThread.cpp src/Base/ThreadedTaskProcessor/WorkerThread.hpp
        src/TWMC/TWMCResults.cpp src/TWMC/TWMCResults.hpp
        src/TWMC/TWMCTypes.h
        src/main.cpp src/Libraries/FilesystemLibrary.h
        src/Libraries/TStaticFactory.h
        src/Base/Utils/StringFormatter.hpp
        src/Base/ThreadedTaskProcessor/WorkerThread.cpp src/Base/ThreadedTaskProcessor/WorkerThread.hpp
        src/Base/Solver.cpp src/Base/Solver.hpp
        src/Base/TaskProcessor.cpp src/Base/TaskProcessor.hpp
        src/Base/Interfaces/ITaskConsumer.hpp src/Base/Interfaces/ITaskConsumer.cpp
        src/Base/ThreadedTaskProcessor/ThreadedTaskProcessor.cpp src/Base/ThreadedTaskProcessor/ThreadedTaskProcessor.hpp
        src/Base/Interfaces/IResultConsumer.cpp src/Base/Interfaces/IResultConsumer.hpp
        src/Base/ResultsSaver.cpp src/Base/ResultsSaver.hpp
        src/TWMC/Solvers/TWMCBaseSolver.cpp src/TWMC/Solvers/TWMCBaseSolver.hpp
        src/TWMC/TWMCTaskData.cpp src/TWMC/TWMCTaskData.hpp
        src/TWMC/TWMCSystemData.cpp src/TWMC/TWMCSystemData.hpp
        src/TWMC/TWMCManager.cpp src/TWMC/TWMCManager.hpp
        src/TWMC/Solvers/TWMCThermoSolver.cpp src/TWMC/Solvers/TWMCThermoSolver.hpp
        src/TWMC/Solvers/TWMCLiebSolver.cpp src/TWMC/Solvers/TWMCLiebSolver.hpp
        src/Libraries/PreAllocator.hpp
        src/Base/ResultsHelpers/HeterogeneousContainer.hpp
        src/Base/TaskResultsGeneric.hpp)

set(GPU_SOURCE_FILES
        src/Base/SolverGPU.cpp src/Base/SolverGPU.hpp
        src/TWMC/Solvers/TWMCLiebGPUSolver.cpp src/TWMC/Solvers/TWMCLiebGPUSolver.hpp
        src/TWMC/Solvers/TWMCBaseGPU.cpp src/TWMC/Solvers/TWMCBaseGPU.hpp)

set(MPI_SOURCE_FILES
        src/Base/MPITaskProcessor/MPINodeManager.cpp src/Base/MPITaskProcessor/MPINodeManager.hpp
        src/Base/MPITaskProcessor/MPIProcessor.cpp src/Base/MPITaskProcessor/MPIProcessor.hpp
        src/Base/Serialization/SerializationArchiveFormats.hpp
        src/TWMC/TWMCSerializators.hpp src/TWMC/TWMCSerializators.cpp src/Base/MPITaskProcessor/MPIPincoTags.hpp
        src/Libraries/eigen_cereal_serialization.hpp src/Base/Serialization/BaseSerializators.cpp)


if (COMPILE_GPU)
    # Append GPU Source FIles
    message("Adding GPU Files")
    set(SOURCE_FILES ${SOURCE_FILES} ${GPU_SOURCE_FILES})
endif(COMPILE_GPU)


if (COMPILE_MPI)
    # Append MPI Source FIles
    message("Adding MPI Files")
    set(SOURCE_FILES ${SOURCE_FILES} ${MPI_SOURCE_FILES})
endif(COMPILE_MPI)

add_executable(sim ${SOURCE_FILES})

# Includes
# include_directories(/usr/local/include)
include_directories(/usr/local/include/eigen3/)

if (LINUX)
    if (COMPILE_MPI)
        include_directories(/home/vicentinif/opt/local/include/)
    endif()
endif()


# Linker
target_link_libraries(sim ${Boost_LIBRARIES})
target_link_libraries(sim ${FFTW_LIBRARIES})
target_link_libraries(sim m)

if (COMPILE_GPU)
    target_link_libraries(sim VexCL::OpenCL)
    target_link_libraries(sim boost_system)
endif(COMPILE_GPU)
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
