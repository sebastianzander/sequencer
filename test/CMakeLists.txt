cmake_minimum_required(VERSION 3.21)
project(test LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test)

set(source_files
    "main.cpp"
)

target_sources(test PRIVATE ${source_files})

target_include_directories(test PUBLIC 
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(test PRIVATE 
    sequencer 
    gmock
    gtest
)

add_custom_command(TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test> $<TARGET_FILE_DIR:test>
    COMMAND_EXPAND_LISTS
)

include(GoogleTest)
gtest_discover_tests(test)
