project(RoachAcquisitionServer)
cmake_minimum_required(VERSION 2.8)

##Add the source files we want to build (best done explicitly)
file(GLOB SRC_LIST "./*.cpp"
    "AVNAppLibs/SocketStreamers/SocketReceiverBase.cpp"
    "AVNAppLibs/SocketStreamers/*.cpp"
    "AVNAppLibs/SocketStreamers/UDPReceiver/*.cpp"
    "AVNAppLibs/SocketStreamers/TCPServer/*.cpp"
    "AVNDataTypes/SpectrometerDataStream/*.cpp"
    "AVNUtilLibs/Sockets/InterruptibleBlockingSockets/*.cpp"
    "AVNUtilLibs/Sockets/InterruptibleBlockingSocketAcceptors/*.cpp")

#If Windows link Boost and HDF5 statically so we don't have to distribute DLLs
if(WIN32)
    message(STATUS "Detected Windows. Setting Boost and HDF5 libs to be linked statically.")
        set(Boost_USE_STATIC_LIBS ON)
        set(HDF5_USE_STATIC_LIBRARIES ON)
endif()

#The Boost libraries
find_package(Boost COMPONENTS system thread REQUIRED)

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
  target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${HDF5_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} hdf5_hl.so)
endif()

## Set the directory where the binaries will be stored
set(EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory where all executables will be stored"
)

#Check for build configuration
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build configuration specified, defaulting to Release")
        set(CMAKE_BUILD_TYPE "Release")
endif()

message(STATUS "Make generation finished for build type " ${CMAKE_BUILD_TYPE})
