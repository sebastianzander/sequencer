cmake_minimum_required(VERSION 3.21)
project(demo LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(YAML_CPP_STATIC_DEFINE)

add_executable(demo)

set(source_files
    "main.cpp"
    "serializer.cpp"
)

target_sources(demo PRIVATE ${source_files})

target_include_directories(demo PUBLIC 
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/cxxopts/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/fmt/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/yaml-cpp/include"
)

target_link_libraries(demo PRIVATE 
    sequencer 
    fmt 
    yaml-cpp
    muparser
)

add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:demo> $<TARGET_FILE_DIR:demo>
    COMMAND_EXPAND_LISTS
)
