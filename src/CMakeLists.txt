cmake_minimum_required(VERSION 3.0.0)
project(FastIBS VERSION 0.1.0)

# Set C++ standard and build type
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")

# Ignore BOOST deprecated headers
add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
add_definitions("-DBOOST_BIND_GLOBAL_PLACEHOLDERS")


# Prefer pthread for Threads package
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS iostreams)


# Add executable targets
add_executable(fastibs FastIBS.cpp)
add_executable(fastibsmapper FastIBSMapper.cpp)
add_executable(KDBIntersect KDBIntersect.cpp)


# Add path to KMC library
set(KMC_LIB_PATH "/project/KMC/bin/libkmc_core.a")

# Link libraries to the executable targets
target_link_libraries(fastibs PRIVATE ZLIB::ZLIB Threads::Threads Boost::boost  ${KMC_LIB_PATH})
target_link_libraries(fastibsmapper PRIVATE ZLIB::ZLIB Threads::Threads Boost::boost  ${KMC_LIB_PATH})
target_link_libraries(KDBIntersect PRIVATE ZLIB::ZLIB Threads::Threads Boost::boost  ${KMC_LIB_PATH})



# Package information
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
