cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL "CMake toolchain file is not set. Set it to vcpk's toolchain file")
endif()

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project(Pixie VERSION 0.3)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_SHARED_LIBS OFF)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")

find_package(fmt CONFIG REQUIRED)
find_package(Lua REQUIRED)
enable_testing()
find_package(GTest CONFIG REQUIRED)
find_package(Freetype REQUIRED)

file(GLOB_RECURSE PIXIE_SOURCES src/*.cpp)

add_library(Pixie STATIC ${PIXIE_SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET Pixie PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_include_directories(Pixie 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Lua_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Pixie PUBLIC
    Freetype::Freetype
    fmt::fmt
    lua
    d3d11
    dxgi
    d3dcompiler
)

##############
# Sandbox

add_executable(SandBox sandbox/sandbox.cpp)
target_link_libraries(SandBox PUBLIC Pixie PRIVATE ${LUA_LIBRARIES})
set_target_properties(SandBox PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:WinMainCRTStartup")
target_include_directories(SandBox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

SET_TARGET_PROPERTIES(SandBox PROPERTIES LINK_FLAGS_DEBUG "/WHOLEARCHIVE:Pixie")
SET_TARGET_PROPERTIES(SandBox PROPERTIES LINK_FLAGS_RELEASE "/WHOLEARCHIVE:Pixie")

################
# Tests

enable_testing()
set(CMAKE_CTEST_COMMAND ctest)
set(CMAKE_CTEST_COMMAND_OPTIONS "--force-new-ctest-process")

add_subdirectory(tests)