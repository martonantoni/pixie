cmake_minimum_required(VERSION 3.0)
project(PixieLib)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")

# Set the output directory for built executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories for header files
#include_directories(${CMAKE_SOURCE_DIR}/include )

# Add subdirectories for libraries
add_subdirectory(system)
add_subdirectory(Flow)
add_subdirectory(pixie)

# Add your main executable or target here
#add_executable(your_executable_name )

# Link the libraries to your executable
#target_link_libraries(your_executable_name library1 library2)
# Add more target_link_libraries() calls if needed for additional libraries