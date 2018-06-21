cmake_minimum_required(VERSION 3.3)
project(Simulator VERSION 0.1 LANGUAGES CXX)

message("Compiling MPI var: ${COMPILE_MPI}")

option(COMPILE_GPU "Build with GPU Support for the Solvers" OFF)
option(COMPILE_MPI "Build with MPI Support for the Manager" OFF)
option(PROFILE_MPI "Build with MPE Profiling support for MPI" OFF)

set(CMAKE_CXX_FLAGS -g)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -DEIGEN_NO_DEBUG")
 #-stdlib=libc++)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #add_definitions(-stdlib=libc++)
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

If(LINUX)
    message(Linux Detected)
    message(is: ${CMAKE_CXX_FLAGS})

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -stdlib=libc++")
        add_definitions(-std=c++17)
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
        add_definitions(-std=c++17)
    endif()
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    if(NOT CEREAL_ROOT)
        set(CEREAL_ROOT /usr/local/include/)
    endif()
endif()

# Directory with all compilation modules
set(CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake-modules)

# Get Boost Packages. Adding the filesystem library on mac because we have no C++17 TS support
if (APPLE)
    set(BOOST_PACKAGES system filesystem)
endif()

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
    find_package(MPI REQUIRED)
    add_definitions(-DMPI_SUPPORT)

    # New interface
    find_package(cereal REQUIRED)
    add_library(cereal UNKNOWN IMPORTED)

    # Find the mpi implementation linker flags and libraries (this is not the best way
    # because I should be changing the compiler...
    if (PROFILE_MPI)
        execute_process(COMMAND mpecc -mpilog --showme:compile OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE MPI_INCLUDE_PATH)
        execute_process(COMMAND mpecc -mpilog --showme:link OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE MPI_LIBRARIES)
    endif()
    message("MPIC++ Compiler flags: ${MPI_INCLUDE_PATH}")
    message("MPIC++ Linker flags: ${MPI_LIBRARIES}")

    # Append compile flags
    #set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ${mpi_compile_flags} ")

    # Append Linker flags (I could also set flags per-target...
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${mpi_linker_flags}")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lompitrace")
    #message("MPIC++ Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")

    #set_target_properties(sim PROPERTIES LINK_FLAGS "${mpi_linker_flags}")
endif (COMPILE_MPI)

# Find package boost, if present
if(APPLE)
    find_package(Boost 1.62 COMPONENTS "${BOOST_PACKAGES}")
    if (NOT Boost_FOUND)
        message(Searching again for Boost)
        SET(BOOST_ROOT ~/opt/local/)
        SET(BOOST_ROOT ~/opt/local/)
        find_package(Boost 1.6 COMPONENTS "${BOOST_PACKAGES}" REQUIRED )
    endif (NOT Boost_FOUND)
elseif(BOOST_PACKAGES)
    find_package(Boost 1.59 COMPONENTS "${BOOST_PACKAGES}" )
    if (NOT Boost_FOUND)
        message("Searching again for Boost without filesystem support")
        SET(BOOST_ROOT ~/opt/local/)
        SET(BOOST_ROOT ~/opt/local/)
        find_package(Boost 1.59 COMPONENTS "${BOOST_PACKAGES}" REQUIRED )
    endif (NOT Boost_FOUND)
endif()

include_directories(${Boost_INCLUDE_DIR})

message("found Boost_INCLUDE_DIR=${Boost_INCLUDE_DIR}")
message("found Boost_LIBRARIES=${Boost_LIBRARIES}")

find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDES})

message("found FFTW_INCLUDE_DIR=${FFTW_INCLUDES}")
message("found FFTW_LIBRARIES=${FFTW_LIBRARIES}")



set(LINKING STATIC)

# Src Folders
include_directories(src)

set(SOURCE_FILES
        Dependencies/easylogging++.cc Dependencies/easylogging++.h
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
        src/Base/Modules/ProgressReporter.cpp src/Base/Modules/ProgressReporter.hpp
        src/Libraries/concurrentqueue.h
        src/Base/TaskProcessors/ThreadedTaskProcessor/WorkerThread.cpp src/Base/TaskProcessors/ThreadedTaskProcessor/WorkerThread.hpp
        src/TWMC/TWMCResults.cpp src/TWMC/TWMCResults.hpp
        src/TWMC/TWMCTypes.h
        src/main.cpp src/Libraries/FilesystemLibrary.h
        src/Libraries/TStaticFactory.h
        src/Base/Utils/StringFormatter.hpp
        src/Base/TaskProcessors/ThreadedTaskProcessor/WorkerThread.cpp src/Base/TaskProcessors/ThreadedTaskProcessor/WorkerThread.hpp
        src/Base/Solver.hpp
        src/Base/TaskProcessor.cpp src/Base/TaskProcessor.hpp
        src/Base/Interfaces/ITaskConsumer.hpp src/Base/Interfaces/ITaskConsumer.cpp
        src/Base/TaskProcessors/ThreadedTaskProcessor/ThreadedTaskProcessor.cpp src/Base/TaskProcessors/ThreadedTaskProcessor/ThreadedTaskProcessor.hpp
        src/Base/Interfaces/IResultConsumer.cpp src/Base/Interfaces/IResultConsumer.hpp
        src/Base/Modules/ResultsSaver.cpp src/Base/Modules/ResultsSaver.hpp
        src/TWMC/TWMCTaskData.cpp src/TWMC/TWMCTaskData.hpp
        src/TWMC/TWMCSystemData.cpp src/TWMC/TWMCSystemData.hpp
        src/TWMC/TWMCManager.cpp src/TWMC/TWMCManager.hpp
        src/TWMC/Solvers/TWMCBaseSolver.cpp src/TWMC/Solvers/TWMCBaseSolver.hpp
        #src/TWMC/Solvers/TWMCThermoSolver.cpp src/TWMC/Solvers/TWMCThermoSolver.hpp
        src/TWMC/Solvers/TWMCLiebSolver.cpp src/TWMC/Solvers/TWMCLiebSolver.hpp
        src/Libraries/PreAllocator.hpp
        src/Base/ResultsHelpers/HeterogeneousContainer.hpp
        src/Base/TaskResultsGeneric.hpp src/Base/SettingStrings.hpp
        src/Base/ManagerUtils/ResortManager.cpp src/Base/ManagerUtils/ResortManager.hpp src/Base/Utils/VectorUtils.hpp src/Base/Utils/Misc.hpp src/Base/Exceptions/NotImplementedException.hpp src/Base/Exceptions/FileNotExistsException.hpp src/Base/Utils/File.cpp src/Base/Utils/File.hpp src/Libraries/enum.h src/Base/ManagerUtils/MergeManager.cpp src/Base/ManagerUtils/MergeManager.hpp)

set(GPU_SOURCE_FILES
        src/Base/SolverGPU.hpp
        src/TWMC/Solvers/TWMCLiebGPUSolver.cpp src/TWMC/Solvers/TWMCLiebGPUSolver.hpp
        src/TWMC/Solvers/TWMCBaseGPU.cpp src/TWMC/Solvers/TWMCBaseGPU.hpp)

set(MPI_SOURCE_FILES
        src/Base/TaskProcessors/MPITaskProcessor/MPINodeManager.cpp src/Base/TaskProcessors/MPITaskProcessor/MPINodeManager.hpp
        src/Base/TaskProcessors/MPITaskProcessor/MPIProcessor.cpp src/Base/TaskProcessors/MPITaskProcessor/MPIProcessor.hpp
        src/Base/Serialization/SerializationArchiveFormats.hpp
        src/TWMC/TWMCSerializators.hpp src/TWMC/TWMCSerializators.cpp src/Base/TaskProcessors/MPITaskProcessor/MPIPincoTags.hpp
        src/Libraries/eigen_cereal_serialization.hpp src/Base/Serialization/BaseSerializators.cpp)


if (COMPILE_GPU)
    # Append GPU Source FIles
    message("Adding GPU Files")
    set(SOURCE_FILES ${SOURCE_FILES} ${GPU_SOURCE_FILES})
endif(COMPILE_GPU)


if (COMPILE_MPI)
    # Append MPI Source FIles
    message("Adding MPI Files")
    set(SOURCE_FILES ${SOURCE_FILES} ${MPI_SOURCE_FILES} )
endif(COMPILE_MPI)

add_executable(sim ${SOURCE_FILES})

find_package(Eigen3 REQUIRED)
add_library(Eigen3 UNKNOWN IMPORTED)
target_include_directories(sim PUBLIC ${EIGEN3_INCLUDES})


# Linker
target_link_libraries(sim ${Boost_LIBRARIES})
target_link_libraries(sim ${FFTW_LIBRARIES}) #target_link_libraries(sim fftw_thread)
target_link_libraries(sim m)

set_target_properties(sim PROPERTIES CXX_STANDARD_REQUIRED 17)

target_include_directories(sim PUBLIC Dependencies)

if (COMPILE_GPU)
    target_link_libraries(sim VexCL::OpenCL)
    target_link_libraries(sim boost_system)
endif(COMPILE_GPU)

if (COMPILE_MPI)
    target_include_directories(sim PUBLIC ${CEREAL_INCLUDES})
    target_include_directories(sim PUBLIC ${MPI_INCLUDE_PATH})
    target_link_libraries(sim ${MPI_LIBRARIES})
endif()

if(LINUX)
    target_link_libraries(sim rt) #realtime

    # On thtop
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(sim c++experimental) #filesystem
    else() #on curie
        target_link_libraries(sim stdc++fs) #filesystem
    endif()
endif()


# Copy file
add_custom_command(TARGET sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:sim>/TestSimulation)

add_custom_command(TARGET sim
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/SampleData/ $<TARGET_FILE_DIR:sim>/TestSimulation/)
