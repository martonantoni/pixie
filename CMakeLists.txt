cmake_minimum_required(VERSION 3.4)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")


project(Pixie VERSION 0.2)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
	  
# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")  # full path in error messages

set(DIRECTX_INCLUDE_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")
set(DIRECTX_LIB_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

# Set the output directory for built executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


#file(GLOB_RECURSE PIXIE_SOURCES src/*.cpp)
file(GLOB_RECURSE PIXIE_SOURCES src/system/*.cpp)
file(GLOB_RECURSE EXCLUDE_SOURCES src/system/lua/*)
list(REMOVE_ITEM PIXIE_SOURCES ${EXCLUDE_SOURCES})

add_library(Pixie ${PIXIE_SOURCES})
target_include_directories(Pixie 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CONAN_INCLUDE_DIRS}
		${DIRECTX_INCLUDE_PATH})

target_link_libraries(Pixie PRIVATE CONAN_PKG::freetype)
		
link_directories(${DIRECTX_LIB_PATH})
		
# Include directories for header files
#include_directories(${CMAKE_SOURCE_DIR}/include )

# Add subdirectories for libraries
#add_subdirectory(system)
#add_subdirectory(Flow)
#add_subdirectory(pixie)

# Add your main executable or target here
#add_executable(your_executable_name )

# Link the libraries to your executable
#target_link_libraries(your_executable_name library1 library2)
# Add more target_link_libraries() calls if needed for additional libraries