cmake_minimum_required(VERSION 3.18)
project(sequencer LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX d)

add_library(sequencer)

set(source_files
    "src/generator.cpp"
    "src/solver.cpp"
    "src/utils.cpp"
)

target_sources(sequencer PRIVATE ${source_files})

target_include_directories(sequencer PRIVATE 
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/include"
)

target_include_directories(sequencer PUBLIC 
    "${CMAKE_SOURCE_DIR}/thirdparty/fmt/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/eigen"
)

target_link_libraries(sequencer PUBLIC fmt)

add_subdirectory("${CMAKE_SOURCE_DIR}/thirdparty")
add_subdirectory("${CMAKE_SOURCE_DIR}/demo")
