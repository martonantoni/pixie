cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL "CMake toolchain file is not set. Set it to vcpk's toolchain file")
endif()

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

# Set the output directory for built executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project(Pixie VERSION 0.3)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_SHARED_LIBS OFF)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")  # full path in error messages
set(DIRECTX_INCLUDE_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")
set(DIRECTX_LIB_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

find_package(directxtk CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Lua REQUIRED)
enable_testing()
find_package(GTest CONFIG REQUIRED)
find_package(Freetype REQUIRED)

file(GLOB_RECURSE PIXIE_SOURCES src/*.cpp)

message(STATUS "FREETYPE_INCLUDE_DIRS: ${FREETYPE_INCLUDE_DIRS}")

add_library(Pixie STATIC ${PIXIE_SOURCES})


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET Pixie PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


target_include_directories(Pixie 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${Lua_INCLUDE_DIRS}
		${DIRECTX_INCLUDE_PATH}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Pixie PUBLIC Microsoft::DirectXTK Freetype::Freetype fmt::fmt lua d3d9.lib d3dx9.lib)
	
link_directories(${DIRECTX_LIB_PATH})

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