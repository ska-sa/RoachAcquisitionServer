project(RoachAcquisitionServer)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE ON)

##Add the source files we want to build (best done explicitly)
file(GLOB SRC_LIST "./*.cpp"
    "AVNAppLibs/SocketStreamers/SocketReceiverBase.cpp"
    "AVNAppLibs/SocketStreamers/*.cpp"
    "AVNAppLibs/SocketStreamers/UDPReceiver/*.cpp"
    "AVNAppLibs/SocketStreamers/TCPServer/*.cpp"
    "AVNAppLibs/KATCP/*.cpp"
    "AVNDataTypes/SpectrometerDataStream/*.cpp"
    "AVNUtilLibs/Timestamp/Timestamp.cpp"
    "AVNUtilLibs/Sockets/InterruptibleBlockingSockets/*.cpp"
    "AVNUtilLibs/Sockets/InterruptibleBlockingSocketAcceptors/*.cpp"
    "AVNUtilLibs/CoordinatePosition/*.cpp"
    "AVNUtilLibs/DirectoryContents/*.cpp"
    "AVNUtilLibs/Filename/*.cpp")

add_definitions(-DUSE_BOOST_TIME) #Use boost library for current time derivation in AVNUtilLibs/Timestamp/Timestamp.cpp
add_definitions(-DKATCP_USE_FLOATS) #Adding floating point support for KATCP sensors

#If Windows link Boost and HDF5 statically so we don't have to distribute DLLs
if(WIN32)
    message(STATUS "Detected Windows. Setting Boost and HDF5 libs to be linked statically.")
        set(Boost_USE_STATIC_LIBS ON)
        set(HDF5_USE_STATIC_LIBRARIES ON)
endif()

#The Boost libraries
find_package(Boost COMPONENTS system thread date_time program_options filesystem REQUIRED)

#HDF5
find_package(HDF5 COMPONENTS C HL REQUIRED)

#The native OS thread library
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC ${PROJECT_NAME} "-pthread")
endif()

#Include and linking directories
include_directories(${Boost_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SRC_LIST})

#Link libraries
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} -lkatcp)
else()
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} -lkatcp)
endif()

## Set the directory where the binaries will be stored
set(EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory where all executables will be stored"
)

set(build_version "unknown")

find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --tags --long --dirty
    WORKING_DIRECTORY "${local_dir}"
    OUTPUT_VARIABLE build_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  message(STATUS "Git hash: ${build_version}")
else()
  message(STATUS "GIT not found")
endif()

add_definitions(-DBUILD_STATE=\"${build_version}\")

#Check for build configuration
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build configuration specified, defaulting to Release")
        set(CMAKE_BUILD_TYPE "Release")
endif()

## Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -std=c++11")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(STATUS "Debug compiler flags are:   " ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS})

elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")  ## Strip binary
    message(STATUS "Release compiler flags are: " ${CMAKE_CXX_FLAGS_Release} ${CMAKE_CXX_FLAGS})

endif()

message(STATUS "Make generation finished for build type " ${CMAKE_BUILD_TYPE})
