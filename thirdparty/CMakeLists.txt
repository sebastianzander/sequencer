cmake_minimum_required(VERSION 3.18)

add_subdirectory(cxxopts)
add_subdirectory(eigen)
add_subdirectory(fmt)

option(ENABLE_SAMPLES "Build the samples" OFF)
set(ENABLE_SAMPLES OFF)
option(ENABLE_OPENMP "Enable OpenMP for multithreading" OFF)
set(ENABLE_OPENMP OFF)
option(ENABLE_WIDE_CHAR "Enable wide character support" OFF)
set(ENABLE_WIDE_CHAR OFF)
option(BUILD_SHARED_LIBS "Build shared/static libs" OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(muparser)

option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
set(YAML_CPP_BUILD_TOOLS OFF)
add_subdirectory(yaml-cpp)
