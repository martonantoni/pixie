cmake_minimum_required(VERSION 3.0)
project(Pixie VERSION 0.2)

find_package(DirectX9 REQUIRED)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")  # full path in error messages

# Set the output directory for built executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


file(GLOB_RECURSE PIXIE_SOURCES src/*.cpp)
file(GLOB_RECURSE EXCLUDE_SOURCES src/system/lua/*)
list(REMOVE_ITEM PIXIE_SOURCES ${EXCLUDE_SOURCES})

add_library(Pixie ${PIXIE_SOURCES})
target_include_directories(Pixie 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include 
	PRIVATE 
		${DirectX9_INCLUDE_DIRS})

target_link_libraries(Pixie PRIVATE ${DirectX9_LIBRARIES})
target_compile_definitions(YourTargetName PRIVATE /D_DIRECTX9_VERSION=0x0900)
	
		
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