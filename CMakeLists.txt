cmake_minimum_required(VERSION 3.4)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
#set(CMAKE_LINK_DEPENDS_NO_SHARED FALSE)
#set(CMAKE_LINK_DEPENDS_NO_SYSTEM FALSE)

# Set the output directory for built executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project(Pixie VERSION 0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")  # full path in error messages
set(DIRECTX_INCLUDE_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")
set(DIRECTX_LIB_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
	  
file(GLOB_RECURSE PIXIE_SOURCES src/system/*.cpp)
file(GLOB_RECURSE EXCLUDE_SOURCES src/system/lua/*)
list(REMOVE_ITEM PIXIE_SOURCES ${EXCLUDE_SOURCES})

add_library(Pixie STATIC ${PIXIE_SOURCES})
target_include_directories(Pixie 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CONAN_INCLUDE_DIRS}
		${DIRECTX_INCLUDE_PATH})

target_link_libraries(Pixie PUBLIC CONAN_PKG::freetype CONAN_PKG::fmt)
		
link_directories(${DIRECTX_LIB_PATH})

add_executable(SandBox sandbox/sandbox.cpp)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:WinMainCRTStartup")
target_link_libraries(SandBox PUBLIC 
	Pixie 
	CONAN_PKG::freetype 
	CONAN_PKG::fmt)
target_include_directories(SandBox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

SET_TARGET_PROPERTIES(SandBox PROPERTIES LINK_FLAGS_DEBUG "/WHOLEARCHIVE:Pixie")
SET_TARGET_PROPERTIES(SandBox PROPERTIES LINK_FLAGS_RELEASE "/WHOLEARCHIVE:Pixie")
		
